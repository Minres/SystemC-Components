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
	
	static final int rulerTickMinorC = 10;
	static final int rulerTickMajorC = 100;

	private int length;
	private long start;
	
	private TxEditorPlugin plugin;
	private Color headerBgColor;
	private Color headerFgColor;
	private int bottom;
	private int baselineY;
	private long scaleFactor=1000000;
	private long rulerScaleFactor=1000000;
	private long rulerTickMinor = rulerTickMinorC*scaleFactor;
	private long rulerTickMajor = rulerTickMajorC*scaleFactor;
	private String unit="";
	
	Ruler(Composite parent, int style) {
		super(parent, style | SWT.DOUBLE_BUFFERED | SWT.NO_BACKGROUND);
		this.length=0;
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
		long startMinorIncr = start;
		long modulo = start % rulerTickMinor;
		startMinorIncr+=rulerTickMinor-modulo;
		long end=start+e.width*scaleFactor;
		
		gc.setBackground(getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND));
		gc.fillRectangle(new Rectangle(e.x, e.y, e.width, height));
		gc.setBackground(headerBgColor);
		gc.fillRectangle(new Rectangle(e.x, e.y, e.width, baselineY));
		gc.setForeground(headerFgColor);
		gc.drawLine(0, bottom, e.width, bottom);
		
		for (long tick = startMinorIncr; tick < end; tick += rulerTickMinor) {
			int x0 = (int) ((tick-start)/scaleFactor);
			if ((tick % rulerTickMajor) == 0) {
				gc.drawText(Double.toString(tick/rulerScaleFactor)+unit, (int) x0, 0);
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

	public void setStartPoint(long l) {
		this.start=l;
		redraw();
	}

	public void setScaleFactor(long scaleFactor) {
		this.scaleFactor=scaleFactor;		
		if(scaleFactor<1000L){
			unit="fs";
			rulerScaleFactor=(long) 1e0;
		}else if(scaleFactor<1000000L){
			unit="ps";			
			rulerScaleFactor=(long) 1e3;
		}else if(scaleFactor<1000000000L){
			unit="ns";			
			rulerScaleFactor=(long) 1e6;
		}else if(scaleFactor<1000000000000L){
			unit="us";			
			rulerScaleFactor=(long) 1e9;
		}else if(scaleFactor<1000000000000000L){
			unit="ms";			
			rulerScaleFactor=(long) 1e9;
		}else{
			unit="s";			
			rulerScaleFactor=(long) 1e12;
		}
		this.rulerTickMinor = rulerTickMinorC*scaleFactor;
		this.rulerTickMajor = rulerTickMajorC*scaleFactor;
		redraw();
	}
}
