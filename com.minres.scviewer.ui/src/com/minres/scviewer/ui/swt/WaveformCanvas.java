/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.ui.swt;

import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.TreeMap;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.graphics.Region;
import org.eclipse.swt.graphics.Transform;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.wb.swt.SWTResourceManager;

import com.minres.scviewer.database.IWaveform;

public class WaveformCanvas extends Canvas {
	public enum Colors {
		LINE,
		LINE_HIGHLITE,
		TRACK_BG_EVEN,
		TRACK_BG_HIGHLITE,
		TRACK_BG_ODD,
		TX_BG,
		TX_BG_HIGHLITE,
		TX_BORDER,
		SIGNAL0,
		SIGNAL1,
		SIGNALZ,
		SIGNALX,
		SIGNAL_TEXT
	}

	Color[] colors=new Color[Colors.values().length];

	/* zooming rates in x and y direction are equal.*/
	final float ZOOMIN_RATE = 1.1f; /* zoomin rate */
	final float ZOOMOUT_RATE = 0.9f; /* zoomout rate */
	
	private int trackHeight = 50;
	private long scaleFactor = 1000000L;
	private long maxTime;
	
	protected Point origin; /* original size */
	protected Transform transform;
	
	protected List<IPainter> painterList;
	TreeMap<Integer, IWaveformPainter> trackVerticalOffset;
	List<IWaveform> streams;

	/**
	 * Constructor for ScrollableCanvas.
	 * @param parent the parent of this control.
	 * @param style the style of this control.
	 */
	public WaveformCanvas(final Composite parent, int style) {
		super( parent, style |SWT.NO_BACKGROUND|SWT.NO_REDRAW_RESIZE|SWT.V_SCROLL|SWT.H_SCROLL);
		addControlListener(new ControlAdapter() { /* resize listener. */
			public void controlResized(ControlEvent event) {
				syncScrollBars();
			}
		});
		addPaintListener(new PaintListener() { /* paint listener. */
			public void paintControl(final PaintEvent event) {
				paint(event.gc);
			}
		});
		painterList=new LinkedList<IPainter>();
		origin=new Point(0,0);
		transform = new Transform(getDisplay());
		trackVerticalOffset=new TreeMap<Integer, IWaveformPainter>();
		initScrollBars();
		initColors(null);
	}

	private void initColors(HashMap<Colors, RGB> colourMap){
		Display d = getDisplay();
		if(colourMap!=null){
			for(Colors c:Colors.values()){
				if(colourMap.containsKey(c)){
					colors[c.ordinal()].dispose();
					colors[c.ordinal()]=new Color(d, colourMap.get(c));
				}
			}
		} else {
			colors[Colors.LINE.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_RED);
			colors[Colors.LINE_HIGHLITE.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_CYAN);
			colors[Colors.TRACK_BG_EVEN.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_BLACK);
			colors[Colors.TRACK_BG_ODD.ordinal()]=SWTResourceManager.getColor(25,25,25);
			colors[Colors.TRACK_BG_HIGHLITE.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_GRAY);
			colors[Colors.TX_BG.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_GREEN);
			colors[Colors.TX_BG_HIGHLITE.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
			colors[Colors.TX_BORDER.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_RED);
			colors[Colors.SIGNAL0.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
			colors[Colors.SIGNAL1.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
			colors[Colors.SIGNALZ.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_GRAY);
			colors[Colors.SIGNALX.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_RED);
			colors[Colors.SIGNAL_TEXT.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_WHITE);
		}	
	}

	public long getMaxTime() {
		return maxTime;
	}

	public void setMaxTime(long maxTime){
		this.maxTime=maxTime;
		syncScrollBars();
	}

	public int getTrackHeight() {
		return trackHeight;
	}

	public void setTrackHeight(int trackHeight) {
		this.trackHeight = trackHeight;
		syncScrollBars();
	}

	public long getScaleFactor() {
		return scaleFactor;
	}

	public void setScaleFactor(long scaleFactor) {
		this.scaleFactor = scaleFactor;
	}

	public void addTrackPainter(IPainter painter) {
		painterList.add(painter);
		redraw();
	}

	public void removeTrackPainter(IPainter painter){
		painterList.remove(painter);
		redraw();
	}

	public void clearAllWavefromPainter() {
		trackVerticalOffset.clear();
		syncScrollBars();
	}

	public void addWavefromPainter(int yoffs, IWaveformPainter painter) {
		trackVerticalOffset.put(yoffs, painter);
		syncScrollBars();
	}

	/**
	 * Dispose the garbage here
	 */
	public void dispose() {
		transform.dispose();
		for(Colors c:Colors.values()) colors[c.ordinal()].dispose();
		super.dispose();
	}

	public void scrollToY(int y){
		ScrollBar vBar = getVerticalBar();
		vBar.setSelection(y);
		scrollVertically(vBar);
	}

	public void scrollToX(int x){
		ScrollBar hBar = getHorizontalBar();
		hBar.setSelection(x);
		scrollHorizontally(hBar);
	}
	
	/* Scroll horizontally */
	private void scrollHorizontally(ScrollBar scrollBar) {
		if (painterList.size()==0) return;
		origin.x= -scrollBar.getSelection();
		syncScrollBars();
	}

	/* Scroll vertically */
	private void scrollVertically(ScrollBar scrollBar) {
		if (painterList.size()==0) return;
		origin.y = -scrollBar.getSelection();
		syncScrollBars();
	}

	/* Initalize the scrollbar and register listeners. */
	private void initScrollBars() {
		ScrollBar horizontal = getHorizontalBar();
		horizontal.setEnabled(false);
		horizontal.setVisible(true);
		horizontal.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent event) {
				scrollHorizontally((ScrollBar) event.widget);
			}
		});
		ScrollBar vertical = getVerticalBar();
		vertical.setEnabled(false);
		vertical.setVisible(true);
		vertical.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent event) {
				scrollVertically((ScrollBar) event.widget);
			}
		});
	}

	/**
	 * Synchronize the scrollbar with the image. If the transform is out
	 * of range, it will correct it. This function considers only following
	 * factors :<b> transform, image size, client area</b>.
	 */
	private void syncScrollBars() {
		if (painterList.size()==0) {
			redraw();
			return;
		}
		int height=1;
		if(trackVerticalOffset.size()>0)
			height=trackVerticalOffset.lastKey()+trackVerticalOffset.lastEntry().getValue().getMinHeight();
		
		int width = (int) (maxTime/scaleFactor);
		ScrollBar horizontal = getHorizontalBar();
		horizontal.setIncrement((int) (getClientArea().width / 100));
		horizontal.setPageIncrement(getClientArea().width);
		int cw = getClientArea().width;
		if (width > cw) { /* image is wider than client area */
			horizontal.setMaximum(width);
			horizontal.setEnabled(true);
			if (((int) - origin.x) > horizontal.getMaximum() - cw)
				origin.x = -horizontal.getMaximum() + cw;
		} else { /* image is narrower than client area */
			horizontal.setEnabled(false);
		}
		horizontal.setSelection(-origin.x);
		horizontal.setThumb(cw);

		ScrollBar vertical = getVerticalBar();
		vertical.setIncrement((int) (getClientArea().height / 100));
		vertical.setPageIncrement((int) (getClientArea().height));
		int ch = getClientArea().height;
		if (height> ch) { /* image is higher than client area */
			vertical.setMaximum(height);
			vertical.setEnabled(true);
			if (((int) - origin.y) > vertical.getMaximum() - ch)
				origin.y = -vertical.getMaximum() + ch;
		} else { /* image is less higher than client area */
			vertical.setMaximum((int) (ch));
			vertical.setEnabled(false);
		}
		vertical.setSelection(-origin.y);
		vertical.setThumb(ch);
		redraw();
	}

	/* Paint function */
	private void paint(GC gc) {
		Rectangle clientRect = getClientArea(); /* Canvas' painting area */
		clientRect.x=-origin.x;
		clientRect.y=-origin.y;
		// reset the transform
		transform.identity();
		// shift the content
		transform.translate(origin.x, origin.y);
		gc.setTransform(transform);
        gc.setClipping(clientRect);
		if (painterList.size()>0 && trackVerticalOffset.size()>0) {
			for(IPainter painter: painterList)
				painter.paintArea(gc, clientRect);
		} else {
			gc.fillRectangle(clientRect);
			initScrollBars();
		}
	}
	
}
