package com.minres.scviewer.ui.swt;

import java.util.Collection;
import java.util.Map;
import java.util.Map.Entry;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Rectangle;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxStream;

class StreamPainter implements IWaveformPainter{

	/**
	 * 
	 */
	private final WaveformCanvas waveCanvas;
	private ITxStream<? extends ITxEvent> stream;
	private int height, upper, txHeight;
	private int totalHeight;
	private boolean even;

	public StreamPainter(WaveformCanvas txDisplay, boolean even, int height, ITxStream<? extends ITxEvent> stream) {
		this.waveCanvas = txDisplay;
		this.stream=stream;
		this.height=height;
		this.upper=this.waveCanvas.getTrackHeight()/5;
		this.txHeight=3*this.waveCanvas.getTrackHeight()/5;
		this.totalHeight=stream.getMaxConcurrency()*this.waveCanvas.getTrackHeight();
		this.even=even;
	}

	public void paintArea(GC gc, Rectangle area) {
		gc.setBackground(this.waveCanvas.colors[even?WaveformCanvas.Colors.TRACK_BG_EVEN.ordinal():WaveformCanvas.Colors.TRACK_BG_ODD.ordinal()]);
		gc.setFillRule(SWT.FILL_EVEN_ODD);
		gc.fillRectangle(area);
		Entry<Long, ?> firstTx=stream.getEvents().floorEntry(area.x*waveCanvas.getScaleFactor());
		Entry<Long, ?> lastTx=stream.getEvents().ceilingEntry((area.x+area.width)*waveCanvas.getScaleFactor());
		if(firstTx==null){
			if(lastTx==null) return;
			firstTx = stream.getEvents().firstEntry();
		} else if(lastTx==null){
			lastTx=stream.getEvents().lastEntry();
		}
        gc.setForeground(this.waveCanvas.colors[WaveformCanvas.Colors.LINE.ordinal()]);
        gc.setBackground(this.waveCanvas.colors[WaveformCanvas.Colors.TX_BG.ordinal()]);
        gc.setFillRule(SWT.FILL_EVEN_ODD);
        gc.setLineStyle(SWT.LINE_SOLID);
        gc.setLineWidth(1);
        for(int y1=area.y+this.waveCanvas.getTrackHeight()/2; y1<area.y+totalHeight; y1+=this.waveCanvas.getTrackHeight())
        	gc.drawLine(area.x, y1, area.x+area.width, y1);
		if(firstTx==lastTx)
			for(ITxEvent x:(Collection<?  extends ITxEvent>)firstTx.getValue())
				drawTx(gc, area, x.getTransaction());					
		else{
			for(Entry<Long, ?> tx: stream.getEvents().subMap(firstTx.getKey(), true, lastTx.getKey(), true).entrySet())
				for(ITxEvent x:(Collection<?  extends ITxEvent>)tx.getValue())
					if(x.getType()==ITxEvent.Type.END) drawTx(gc, area, x.getTransaction());
		}
	}

	protected void drawTx(GC gc, Rectangle area, ITx tx) {
		int offset = tx.getConcurrencyIndex()*this.waveCanvas.getTrackHeight();
		Rectangle bb = new Rectangle(
				(int)(tx.getBeginTime()/this.waveCanvas.getScaleFactor()), area.y+offset+upper,
				(int)((tx.getEndTime()-tx.getBeginTime())/this.waveCanvas.getScaleFactor()), txHeight);
		if(bb.x+bb.width<area.x || bb.x>area.x+area.width) return;
		if(bb.width<10){
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

}