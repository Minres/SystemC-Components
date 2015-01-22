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

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map.Entry;
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
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.graphics.Transform;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.wb.swt.SWTResourceManager;

import com.google.common.collect.Lists;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;

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
		SIGNAL_TEXT,
		CURSOR
	}

	Color[] colors=new Color[Colors.values().length];
	
	private int trackHeight = 50;
	private long scaleFactor = 1000000L;
	private int  level=6;
	private long maxTime;
	protected Point origin; /* original size */
	protected Transform transform;
	protected Ruler ruler;
	protected List<IPainter> painterList;
	TreeMap<Integer, IWaveformPainter> trackVerticalOffset;
	
	protected List<IWaveform<? extends IWaveformEvent>> streams;

	ITx currentSelection;
	IWaveform<? extends IWaveformEvent> currentWaveformSelection;


	/**
	 * Constructor for ScrollableCanvas.
	 * @param parent the parent of this control.
	 * @param style the style of this control.
	 */
	public WaveformCanvas(final Composite parent, int style) {
		super( parent, style |SWT.DOUBLE_BUFFERED| SWT.NO_BACKGROUND|SWT.NO_REDRAW_RESIZE|SWT.V_SCROLL|SWT.H_SCROLL);
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
			colors[Colors.TRACK_BG_ODD.ordinal()]=SWTResourceManager.getColor(40,40,40);
			colors[Colors.TRACK_BG_HIGHLITE.ordinal()]=SWTResourceManager.getColor(40,40,80);
			colors[Colors.TX_BG.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_GREEN);
			colors[Colors.TX_BG_HIGHLITE.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
			colors[Colors.TX_BORDER.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_RED);
			colors[Colors.SIGNAL0.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
			colors[Colors.SIGNAL1.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
			colors[Colors.SIGNALZ.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_GRAY);
			colors[Colors.SIGNALX.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_RED);
			colors[Colors.SIGNAL_TEXT.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_WHITE);
			colors[Colors.CURSOR.ordinal()]=SWTResourceManager.getColor(SWT.COLOR_YELLOW);
		}	
	}

	public List<IWaveform<? extends IWaveformEvent>> getStreams() {
		return streams;
	}

	public void setStreams(List<IWaveform<? extends IWaveformEvent>> streams) {
		this.streams = streams;
	}

	public Ruler getRuler(){
		return ruler;
	}
	
	public void setRuler(Ruler ruler) {
		this.ruler=ruler;
	}

	public Object getOrigin() {
		return origin;
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

	public void setZoomLevel(int level) {
		this.level=level;
		this.scaleFactor = (long) Math.pow(10, level);
		if(ruler!=null)	ruler.setStartPoint(-origin.x*scaleFactor);
		syncScrollBars();
	}

	public int getZoomLevel() {
		return level;
	}
	
	public long getScaleFactor() {
		return scaleFactor;
	}

	public void addPainter(IPainter painter) {
		painterList.add(painter);
		redraw();
	}

	public void removePainter(IPainter painter){
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
		if(ruler!=null)	ruler.setStartPoint(-origin.x*scaleFactor);
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
		ruler.setScaleFactor(scaleFactor);
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

	public Object getClicked(Point point) {
		for(IPainter p:Lists.reverse(painterList)){
			if(p instanceof TrackPainter){
				int y= point.y-origin.y;
				int x=point.x-origin.x;
				Entry<Integer, IWaveformPainter> entry = trackVerticalOffset.floorEntry(y);
				if(entry!=null){
					if(entry.getValue() instanceof StreamPainter){
						return ((StreamPainter)entry.getValue()).getClicked(new Point(x,y-entry.getKey()));
					}else if(entry.getValue() instanceof SignalPainter)
						return ((SignalPainter)entry.getValue()).getSignal();
				}				
			}else if(p instanceof CursorPainter){
				if(Math.abs(point.x*scaleFactor-((CursorPainter)p).getTime())<2){
					return p;
				}
			}
		}
		return null;
	}

	public void setSelected(ITx currentSelection, IWaveform<? extends IWaveformEvent> currentWaveformSelection) {
		this.currentSelection=currentSelection;
		this.currentWaveformSelection=currentWaveformSelection;
		if(currentSelection!=null) reveal(currentSelection.getBeginTime(), currentSelection.getEndTime());
		redraw();
	}

	public void reveal(Long beginTime, Long endTime) {
		int lower = (int) (beginTime/scaleFactor);
		int higher=(int) (endTime/scaleFactor);
		Point size = getSize();
		if(lower<-origin.x){
			scrollToX(lower);
		} else if(higher>(size.x-origin.x)){
			scrollToX(higher-size.x);
		}
	}

}
