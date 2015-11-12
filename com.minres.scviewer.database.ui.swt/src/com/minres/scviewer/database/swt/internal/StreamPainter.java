/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database.swt.internal;

import java.util.Collection;
import java.util.List;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeSet;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.ui.TrackEntry;
import com.minres.scviewer.database.ui.WaveformColors;

public class StreamPainter extends TrackPainter{

	/**
	 * 
	 */
	private final WaveformCanvas waveCanvas;
	private ITxStream<? extends ITxEvent> stream;
	private int txBase, txHeight;
	private boolean even;
	private TreeSet<ITx> seenTx;

	public StreamPainter(WaveformCanvas waveCanvas, boolean even, TrackEntry trackEntry) {
		super(trackEntry, even);
		this.waveCanvas = waveCanvas;
		this.stream=trackEntry.getStream();
		this.seenTx=new TreeSet<ITx>();
	}

	@SuppressWarnings("unchecked")
	public void paintArea(GC gc, Rectangle area) {
		if(stream.getEvents().size()==0) return;
		int trackHeight=trackEntry.height/stream.getMaxConcurrency();
		txBase=trackHeight/5;
		txHeight=trackHeight*3/5;
		if(trackEntry.selected)
			gc.setBackground(this.waveCanvas.colors[WaveformColors.TRACK_BG_HIGHLITE.ordinal()]);
		else
			gc.setBackground(this.waveCanvas.colors[even?WaveformColors.TRACK_BG_EVEN.ordinal():WaveformColors.TRACK_BG_ODD.ordinal()]);
		gc.setFillRule(SWT.FILL_EVEN_ODD);
		gc.fillRectangle(area);
		Entry<Long, ?> firstTx=stream.getEvents().floorEntry(area.x*waveCanvas.getScaleFactor());
		Entry<Long, ?> lastTx=stream.getEvents().ceilingEntry((area.x+area.width)*waveCanvas.getScaleFactor());
		if(firstTx==null) firstTx = stream.getEvents().firstEntry();
		if(lastTx==null) lastTx=stream.getEvents().lastEntry();
        gc.setFillRule(SWT.FILL_EVEN_ODD);
        gc.setLineStyle(SWT.LINE_SOLID);
        gc.setLineWidth(1);
        gc.setForeground(this.waveCanvas.colors[WaveformColors.LINE.ordinal()]);
        for(int y1=area.y+trackHeight/2; y1<area.y+trackEntry.height; y1+=trackHeight)
        	gc.drawLine(area.x, y1, area.x+area.width, y1);
		if(firstTx==lastTx)
			for(ITxEvent txEvent:(Collection<?  extends ITxEvent>)firstTx.getValue())
				drawTx(gc, area, txEvent.getTransaction());					
		else{
			seenTx.clear();
			NavigableMap<Long,?> entries = stream.getEvents().subMap(firstTx.getKey(), true, lastTx.getKey(), true);
			boolean highlighed=false;
	        gc.setForeground(this.waveCanvas.colors[WaveformColors.LINE.ordinal()]);
	        gc.setBackground(this.waveCanvas.colors[WaveformColors.TX_BG.ordinal()]);
			for(Entry<Long, ?> entry: entries.entrySet())
				for(ITxEvent txEvent:(Collection<?  extends ITxEvent>)entry.getValue()){
					if(txEvent.getType()==ITxEvent.Type.BEGIN)
						seenTx.add(txEvent.getTransaction());
					if(txEvent.getType()==ITxEvent.Type.END){
						ITx tx = txEvent.getTransaction();
						highlighed|=waveCanvas.currentSelection!=null && waveCanvas.currentSelection.equals(tx);
						drawTx(gc, area, tx);
						seenTx.remove(tx);
					}
				}
			for(ITx tx:seenTx){
				drawTx(gc, area, tx);
			}
			if(highlighed){
		        gc.setForeground(this.waveCanvas.colors[WaveformColors.LINE_HIGHLITE.ordinal()]);
		        gc.setBackground(this.waveCanvas.colors[WaveformColors.TX_BG_HIGHLITE.ordinal()]);
				drawTx(gc, area, waveCanvas.currentSelection);
			}
		}
	}

	protected void drawTx(GC gc, Rectangle area, ITx tx) {
		int offset = tx.getConcurrencyIndex()*this.waveCanvas.getTrackHeight();
		Rectangle bb = new Rectangle(
				(int)(tx.getBeginTime()/this.waveCanvas.getScaleFactor()), area.y+offset+txBase,
				(int)((tx.getEndTime()-tx.getBeginTime())/this.waveCanvas.getScaleFactor()), txHeight);
		if(bb.x+bb.width<area.x || bb.x>area.x+area.width) return;
		if(bb.width==0){
			gc.drawLine(bb.x, bb.y, bb.x, bb.y+bb.height);
		} else if(bb.width<10){
			gc.fillRectangle(bb);
			gc.drawRectangle(bb);
		} else {
		    gc.fillRoundRectangle(bb.x, bb.y, bb.width, bb.height, 5, 5);
		    gc.drawRoundRectangle(bb.x, bb.y, bb.width, bb.height, 5, 5);
		}
	}

	public ITx getClicked(Point point) {
		int lane=point.y/waveCanvas.getTrackHeight();
		Entry<Long, List<ITxEvent>> firstTx=stream.getEvents().floorEntry(point.x*waveCanvas.getScaleFactor());
		if(firstTx!=null){
			do {
				ITx tx = getTxFromEntry(lane, point.x, firstTx);
				if(tx!=null) return tx;
				firstTx=stream.getEvents().lowerEntry(firstTx.getKey());
			}while(firstTx!=null);
		}
		return null;
	}

	public ITxStream<? extends ITxEvent> getStream() {
		return stream;
	}

	public void setStream(ITxStream<? extends ITxEvent> stream) {
		this.stream = stream;
	}

	protected ITx getTxFromEntry(int lane, int offset, Entry<Long, List<ITxEvent>> firstTx) {
        long timePoint=offset*waveCanvas.getScaleFactor();
		for(ITxEvent evt:firstTx.getValue()){
		    ITx tx=evt.getTransaction();
			if(evt.getType()==ITxEvent.Type.BEGIN && tx.getConcurrencyIndex()==lane && tx.getBeginTime()<=timePoint && tx.getEndTime()>=timePoint){
				return evt.getTransaction();
			}
		}
		// now with some fuzziness
        timePoint=(offset-5)*waveCanvas.getScaleFactor();
        long timePointHigh=(offset+5)*waveCanvas.getScaleFactor();
        for(ITxEvent evt:firstTx.getValue()){
            ITx tx=evt.getTransaction();
            if(evt.getType()==ITxEvent.Type.BEGIN && tx.getConcurrencyIndex()==lane && tx.getBeginTime()<=timePointHigh && tx.getEndTime()>=timePoint){
                return evt.getTransaction();
            }
        }
		return null;
	}

}