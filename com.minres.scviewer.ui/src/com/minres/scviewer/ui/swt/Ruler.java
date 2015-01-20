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

public class Ruler extends Composite {

	static final int height = 20;
	static final int tickY = 15;
	static final int majorTickY = 5;
	
	static final int rulerTickMinor = 10;
	static final int rulerTickMajor = 100;

	private int length;
	private int start;
	
	private TxEditorPlugin plugin;
	private Color headerBgColor;
	private Color headerFgColor;
	private int bottom;
	private int baselineY;

	Ruler(Composite parent, int style, int lenght) {
		super(parent, style | SWT.DOUBLE_BUFFERED);
		this.length=lenght;
		headerBgColor=getDisplay().getSystemColor(SWT.COLOR_WHITE);
		headerFgColor=getDisplay().getSystemColor(SWT.COLOR_BLACK);
		plugin=TxEditorPlugin.getDefault();	
		if(plugin!=null){
			headerBgColor=plugin.getColor(TxEditorPlugin.headerBgColor);
			headerFgColor=plugin.getColor(TxEditorPlugin.headerFgColor);
		}
		addDisposeListener(new DisposeListener() {
			public void widgetDisposed(DisposeEvent e) {
				Ruler.this.widgetDisposed(e);
			}
		});
		addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				Ruler.this.paintControl(e);
			}
		});
		
		bottom=height - 2;
		baselineY=height - 1;
	}

	public int getLength() {
		return length;
	}

	public void setLength(int length) {
		this.length = length;
		layout(true);
		redraw();
	}

	protected void widgetDisposed(DisposeEvent e) {
	}
	
	void paintControl(PaintEvent e) {
		GC gc = e.gc;
		int startMinorIncr = start;
		int modulo = start % rulerTickMinor;
		startMinorIncr+=rulerTickMinor-modulo;
		int end=start+e.width;
		
		gc.setBackground(getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND));
		gc.fillRectangle(new Rectangle(e.x, e.y, e.width, height));
		gc.setBackground(headerBgColor);
		gc.fillRectangle(new Rectangle(e.x, e.y, e.width, baselineY));
		gc.setForeground(headerFgColor);
		gc.drawLine(0, bottom, e.width, bottom);
		
		for (int tick = startMinorIncr; tick < end; tick += rulerTickMinor) {
			int x0 = tick-start;
			if ((tick % rulerTickMajor) == 0) {
				gc.drawText(Integer.toString(tick), x0, 0);
				gc.drawLine(x0, majorTickY, x0, bottom);
			} else {
				gc.drawLine(x0, tickY, x0, bottom);
			}
		}
	}

	@Override
	public Point computeSize(int wHint, int hHint, boolean changed) {
		return new Point(0, height);
	}

	public void setStartPoint(int start) {
		this.start=start;
		redraw();
	}
}
