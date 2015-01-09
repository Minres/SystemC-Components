package com.minres.scviewer.ui.swt;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Composite;

import com.minres.scviewer.ui.TxEditorPlugin;

public class Transaction extends Composite {

	public static int height = 50;
	public static Color lineColor;
	public static Color txBgColor;
	public static Color highliteLineColor;
	public static Color txHighliteBgColor;
	public static Color trackBgColor;
	private int length;
	private boolean highlighted=false;
	
	Transaction(Composite parent, int style, int lenght) {
		super(parent, style|SWT.NO_BACKGROUND);
		this.length=lenght;
		addDisposeListener(new DisposeListener() {
			public void widgetDisposed(DisposeEvent e) {
				Transaction.this.widgetDisposed(e);
			}
		});
		addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				Transaction.this.paintControl(e);
			}
		});
		TxEditorPlugin plugin=TxEditorPlugin.getDefault();	
		lineColor=plugin.getColor(TxEditorPlugin.lineColor);
		txBgColor=plugin.getColor(TxEditorPlugin.txBgColor);
		trackBgColor=plugin.getColor(TxEditorPlugin.trackBgDarkColor);
		highliteLineColor=plugin.getColor(TxEditorPlugin.highliteLineColor);
		txHighliteBgColor=plugin.getColor(TxEditorPlugin.txHighliteBgColor);
		setBackground(trackBgColor);
	}

	protected void widgetDisposed(DisposeEvent e) {
	}
	
	void paintControl(PaintEvent e) {
		GC gc = e.gc;
        gc.setBackground(trackBgColor);
        gc.fillRectangle(new Rectangle(0, 0, length, height));
        gc.setForeground(highlighted?highliteLineColor:lineColor);
        gc.setFillRule(SWT.FILL_EVEN_ODD);
        gc.setBackground(highlighted?txHighliteBgColor:txBgColor);
        gc.setLineWidth(1);
        gc.setLineStyle(SWT.LINE_SOLID);
		Rectangle bb = new Rectangle(0, height/5, length-1, 3*height/5);
		if(bb.width<10){
			gc.fillRectangle(bb);
			gc.drawRectangle(bb);
		} else {
	        gc.fillRoundRectangle(bb.x, bb.y, bb.width, bb.height, 5, 5);
	        gc.drawRoundRectangle(bb.x, bb.y, bb.width, bb.height, 5, 5);
		}	
	}

	@Override
	public Point computeSize(int wHint, int hHint, boolean changed) {
		return new Point(length, height);
	}

	public void highlight(boolean highlight) {
		highlighted=highlight;
		redraw();
	}
}
