/*******************************************************************************
 * Copyright (c) 2014, 2015 MINRES Technologies GmbH and others.
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

public class StreamPainter implements IWaveformPainter{

	/**
	 * 
	 */
	private final WaveformCanvas waveCanvas;
	private ITxStream<? extends ITxEvent> stream;
	private int height, upper, txHeight;
	private int totalHeight;
	private boolean even;
	private TreeSet<ITx> seenTx;

	public StreamPainter(WaveformCanvas txDisplay, boolean even, int height, ITxStream<? extends ITxEvent> stream) {
		this.waveCanvas = txDisplay;
		this.stream=stream;
		this.height=height;
		this.upper=this.waveCanvas.getTrackHeight()/5;
		this.txHeight=3*this.waveCanvas.getTrackHeight()/5;
		this.totalHeight=stream.getMaxConcurrency()*this.waveCanvas.getTrackHeight();
		this.even=even;
		this.seenTx=new TreeSet<ITx>();
	}

	@SuppressWarnings("unchecked")
	public void paintArea(GC gc, Rectangle area) {
		if(stream.getEvents().size()==0) return;
		if(waveCanvas.currentWaveformSelection!=null && waveCanvas.currentWaveformSelection.getId()==stream.getId())
			gc.setBackground(this.waveCanvas.colors[WaveformCanvas.Colors.TRACK_BG_HIGHLITE.ordinal()]);
		else
			gc.setBackground(this.waveCanvas.colors[even?WaveformCanvas.Colors.TRACK_BG_EVEN.ordinal():WaveformCanvas.Colors.TRACK_BG_ODD.ordinal()]);
		gc.setFillRule(SWT.FILL_EVEN_ODD);
		gc.fillRectangle(area);
		Entry<Long, ?> firstTx=stream.getEvents().floorEntry(area.x*waveCanvas.getScaleFactor());
		Entry<Long, ?> lastTx=stream.getEvents().ceilingEntry((area.x+area.width)*waveCanvas.getScaleFactor());
		if(firstTx==null) firstTx = stream.getEvents().firstEntry();
		if(lastTx==null) lastTx=stream.getEvents().lastEntry();
        gc.setFillRule(SWT.FILL_EVEN_ODD);
        gc.setLineStyle(SWT.LINE_SOLID);
        gc.setLineWidth(1);
        gc.setForeground(this.waveCanvas.colors[WaveformCanvas.Colors.LINE.ordinal()]);
        for(int y1=area.y+this.waveCanvas.getTrackHeight()/2; y1<area.y+totalHeight; y1+=this.waveCanvas.getTrackHeight())
        	gc.drawLine(area.x, y1, area.x+area.width, y1);
		if(firstTx==lastTx)
			for(ITxEvent txEvent:(Collection<?  extends ITxEvent>)firstTx.getValue())
				drawTx(gc, area, txEvent.getTransaction());					
		else{
			seenTx.clear();
			NavigableMap<Long,?> entries = stream.getEvents().subMap(firstTx.getKey(), true, lastTx.getKey(), true);
			boolean highlighed=false;
	        gc.setForeground(this.waveCanvas.colors[WaveformCanvas.Colors.LINE.ordinal()]);
	        gc.setBackground(this.waveCanvas.colors[WaveformCanvas.Colors.TX_BG.ordinal()]);
			for(Entry<Long, ?> entry: entries.entrySet())
				for(ITxEvent txEvent:(Collection<?  extends ITxEvent>)entry.getValue()){
					if(txEvent.getType()==ITxEvent.Type.BEGIN)
						seenTx.add(txEvent.getTransaction());
					if(txEvent.getType()==ITxEvent.Type.END){
						ITx tx = txEvent.getTransaction();
						highlighed|=waveCanvas.currentSelection!=null && waveCanvas.currentSelection.getId()==tx.getId();
						drawTx(gc, area, tx);
						seenTx.remove(tx);
					}
				}
			for(ITx tx:seenTx){
				drawTx(gc, area, tx);
			}
			if(highlighed){
		        gc.setForeground(this.waveCanvas.colors[WaveformCanvas.Colors.LINE_HIGHLITE.ordinal()]);
		        gc.setBackground(this.waveCanvas.colors[WaveformCanvas.Colors.TX_BG_HIGHLITE.ordinal()]);
				drawTx(gc, area, waveCanvas.currentSelection);
			}
		}
	}

	protected void drawTx(GC gc, Rectangle area, ITx tx) {
		int offset = tx.getConcurrencyIndex()*this.waveCanvas.getTrackHeight();
		Rectangle bb = new Rectangle(
				(int)(tx.getBeginTime()/this.waveCanvas.getScaleFactor()), area.y+offset+upper,
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

	@Override
	public int getMinHeight() {
		return height;
	}

	public Object getClicked(Point point) {
		int lane=point.y/waveCanvas.getTrackHeight();
		Entry<Long, List<ITxEvent>> firstTx=stream.getEvents().floorEntry(point.x*waveCanvas.getScaleFactor());
		if(firstTx!=null){
			do {
				ITx tx = getTxFromEntry(lane, firstTx);
				if(tx!=null) return tx;
				firstTx=stream.getEvents().lowerEntry(firstTx.getKey());
			}while(firstTx!=null);
		}
		return stream;
	}

	public ITxStream<? extends ITxEvent> getStream() {
		return stream;
	}

	public void setStream(ITxStream<? extends ITxEvent> stream) {
		this.stream = stream;
	}

	protected ITx getTxFromEntry(int lane, Entry<Long, List<ITxEvent>> firstTx) {
		for(ITxEvent evt:firstTx.getValue()){
			if(evt.getType()==ITxEvent.Type.BEGIN && evt.getTransaction().getConcurrencyIndex()==lane){
				return evt.getTransaction();
			}
		}
		return null;
	}

}