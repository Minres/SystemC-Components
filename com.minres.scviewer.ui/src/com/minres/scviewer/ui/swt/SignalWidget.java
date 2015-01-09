package com.minres.scviewer.ui.swt;

import java.util.NavigableSet;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.wb.swt.SWTResourceManager;

import com.minres.scviewer.database.EventTime;
import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ISignalChange;
import com.minres.scviewer.database.ISignalChangeMulti;
import com.minres.scviewer.database.ISignalChangeSingle;
import com.minres.scviewer.ui.TxEditorPlugin;

public class SignalWidget extends Canvas implements IWaveformWidget{

	static final int trackHeight = 50;
	static final int trackInset = 1;
	static final int txHeight = trackHeight - 2 * trackInset;

	static double zoomFactor = EventTime.getScalingFactor(EventTime.Unit.NS);
	private Color lineColor;
	private Color trackBgColor;
	private Color color0;
	private Color color1;
	private Color colorZ;
	private Color colorX;
	private Color colorText;
	private long length;
	ISignal<ISignalChange> signal;
	
	public SignalWidget(Composite parent, int style) {
		super(parent, style);
		addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				SignalWidget.this.paintControl(e);
			}
		});
		TxEditorPlugin plugin=TxEditorPlugin.getDefault();	
		lineColor=plugin.getColor(TxEditorPlugin.lineColor);
		trackBgColor=plugin.getColor(TxEditorPlugin.trackBgDarkColor);
		color0=SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
		color1=SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
		colorZ=SWTResourceManager.getColor(SWT.COLOR_GRAY);
		colorX=SWTResourceManager.getColor(SWT.COLOR_RED);
		colorText=SWTResourceManager.getColor(SWT.COLOR_WHITE);
	}

	public void setTransactions(ISignal<ISignalChange> signal) {
		this.signal=signal;
		ISignalChange change = signal.getSignalChanges().last();
		length=(long) (change.getTime().getValue()/zoomFactor);
		layout(true,true);
	}

	@Override
	public Point computeSize (int wHint, int hHint, boolean changed) {
		return new Point((int) length, trackHeight);		
	}

	void paintControl(PaintEvent e) {
		GC gc = e.gc;
		gc.setForeground(lineColor);
		gc.setFillRule(SWT.FILL_EVEN_ODD);
		gc.setBackground(trackBgColor);
		gc.setLineWidth(1);
		gc.setLineStyle(SWT.LINE_SOLID);
		gc.fillRectangle(new Rectangle(e.x, e.y, e.width, e.height));
		ISignalChange lastChange = null;
		NavigableSet<ISignalChange> visibleChanges = signal.getSignalChangesByTimes(
				new EventTime((long) (e.x*zoomFactor)), 
				new EventTime((long) ((e.x+e.width)*zoomFactor)));
		for(ISignalChange actChange:visibleChanges){
			if(lastChange!=null){
				drawValues(e, gc, lastChange, actChange);
			}
			lastChange=actChange;
		}
	}

	protected void drawValues(PaintEvent e, GC gc, ISignalChange lastChange, ISignalChange actChange) {
		if(lastChange instanceof ISignalChangeSingle){
			int yOffset = trackHeight/2;
			Color color = colorX;
			switch(((ISignalChangeSingle) lastChange).getValue()){
			case '1':
				color=color1;
				yOffset = trackHeight/5;
				break;
			case '0':
				color=color0;
				yOffset = 4*trackHeight/5;
				break;
			case 'Z':
				color=colorZ;
				break;
			default:	
			}
			gc.setForeground(color);
			int endTime= (int)(actChange.getTime().getValue()/zoomFactor);
			gc.drawLine((int)(lastChange.getTime().getValue()/zoomFactor), yOffset,	endTime, yOffset);
			int yNext =  trackHeight/2;
			switch(((ISignalChangeSingle) actChange).getValue()){
			case '1':
				yNext = trackHeight/5;
				break;
			case '0':
				yNext = 4*trackHeight/5;
				break;
			default:	
			}
//			gc.setForeground(colorC);
			if(yOffset<yNext)
				gc.drawLine(endTime, yOffset, endTime, yNext);
			else
				gc.drawLine(endTime, yNext, endTime, yOffset);
			
		} else if(lastChange instanceof ISignalChangeMulti){
			int yOffsetT = trackHeight/5;
			int yOffsetM = trackHeight/2;
			int yOffsetB = 4*trackHeight/5;
			Color colorBorder = color0;
			ISignalChangeMulti last = (ISignalChangeMulti) lastChange;
			if(last.getValue().toString().contains("X")){
				colorBorder=colorX;
			}else if(last.getValue().toString().contains("Z")){
				colorBorder=colorZ;
			}
			int beginTime= (int)(lastChange.getTime().getValue()/zoomFactor);
			int endTime= (int)(actChange.getTime().getValue()/zoomFactor);
			int[] points = {
					beginTime,yOffsetM, 
					beginTime+1,yOffsetT, 
					endTime-1,yOffsetT, 
					endTime,yOffsetM, 
					endTime-1,yOffsetB, 
					beginTime+1,yOffsetB
			};
			gc.setForeground(colorBorder);
			gc.drawPolygon(points);
			gc.setForeground(colorText);
			int size = gc.getDevice().getDPI().y * gc.getFont().getFontData()[0].getHeight()/72;
//			gc.setClipping(beginTime+3,yOffsetM-size/2-1,endTime-beginTime-4, yOffsetM+size/2+1); 
			gc.drawText("h'"+last.getValue().toHexString(), beginTime+3, yOffsetM-size/2-1);
		}
	}

	@Override
	public Transaction highlight(Object sel) {
		// TODO Auto-generated method stub
		return null;
	}
}
