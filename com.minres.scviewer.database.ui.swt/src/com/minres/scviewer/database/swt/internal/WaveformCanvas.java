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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map.Entry;

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
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.wb.swt.SWTResourceManager;

import com.google.common.collect.Lists;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.RelationType;
import com.minres.scviewer.database.ui.IWaveformViewer;
import com.minres.scviewer.database.ui.WaveformColors;

public class WaveformCanvas extends Canvas {
	
    Color[] colors = new Color[WaveformColors.values().length];

    private int trackHeight = 50;
    
    private long scaleFactor = 1000000L; // 1ns
    
    String unit="ns";
    
    private int level = 12;
    
    public final static String[] unitString={"fs", "ps", "ns", "µs", "ms"};//, "s"};
    
    public final static int[] unitMultiplier={1, 3, 10, 30, 100, 300};
    
    private long maxTime;
    
    protected Point origin; /* original size */
    
    protected Transform transform;
    
    protected int rulerHeight=40;
    
    protected List<IPainter> painterList;
    
    ITx currentSelection;
    
    private List<SelectionAdapter> selectionListeners;

	private RulerPainter rulerPainter;

	private TrackAreaPainter trackAreaPainter;

	private ArrowPainter arrowPainter;

	private List<CursorPainter> cursorPainters;

	HashMap<IWaveform<?>, IWaveformPainter> wave2painterMap;
    /**
     * Constructor for ScrollableCanvas.
     * 
     * @param parent
     *            the parent of this control.
     * @param style
     *            the style of this control.
     */
    public WaveformCanvas(final Composite parent, int style) {
        super(parent, style | SWT.DOUBLE_BUFFERED | SWT.NO_BACKGROUND | SWT.NO_REDRAW_RESIZE | SWT.V_SCROLL | SWT.H_SCROLL);
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
        painterList = new LinkedList<IPainter>();
        origin = new Point(0, 0);
        transform = new Transform(getDisplay());
        selectionListeners = new LinkedList<>();
        cursorPainters= new ArrayList<>();
        wave2painterMap=new HashMap<>();
        
        initScrollBars();
        initColors(null);
		// order is important: it is bottom to top
        trackAreaPainter=new TrackAreaPainter(this);
        painterList.add(trackAreaPainter);
        rulerPainter=new RulerPainter(this);
        painterList.add(rulerPainter);
        arrowPainter=new ArrowPainter(this, IWaveformViewer.NEXT_PREV_IN_STREAM);
        painterList.add(arrowPainter);
		CursorPainter cp = new CursorPainter(this, scaleFactor * 10, cursorPainters.size()-1);
		painterList.add(cp);
		cursorPainters.add(cp);
		CursorPainter marker = new CursorPainter(this, scaleFactor * 100, cursorPainters.size()-1);
		painterList.add(marker);
		cursorPainters.add(marker);
		wave2painterMap=new HashMap<>();
    }

	public void addCursoPainter(CursorPainter cursorPainter){
		painterList.add(cursorPainter);
		cursorPainters.add(cursorPainter);
	}
	
    public void initColors(HashMap<WaveformColors, RGB> colourMap) {
        Display d = getDisplay();
        if (colourMap != null) {
            for (WaveformColors c : WaveformColors.values()) {
                if (colourMap.containsKey(c)) {
                    colors[c.ordinal()] = new Color(d, colourMap.get(c));
                }
            }
            redraw();
        } else {
            colors[WaveformColors.LINE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
            colors[WaveformColors.LINE_HIGHLITE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_CYAN);
            colors[WaveformColors.TRACK_BG_EVEN.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_BLACK);
            colors[WaveformColors.TRACK_BG_ODD.ordinal()] = SWTResourceManager.getColor(40, 40, 40);
            colors[WaveformColors.TRACK_BG_HIGHLITE.ordinal()] = SWTResourceManager.getColor(40, 40, 80);
            colors[WaveformColors.TX_BG.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GREEN);
            colors[WaveformColors.TX_BG_HIGHLITE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
            colors[WaveformColors.TX_BORDER.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
            colors[WaveformColors.SIGNAL0.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
            colors[WaveformColors.SIGNAL1.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
            colors[WaveformColors.SIGNALZ.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GRAY);
            colors[WaveformColors.SIGNALX.ordinal()] = SWTResourceManager.getColor(255,  128,  182);
            colors[WaveformColors.SIGNAL_TEXT.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_WHITE);
            colors[WaveformColors.CURSOR.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
            colors[WaveformColors.CURSOR_DRAG.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GRAY);
            colors[WaveformColors.CURSOR_TEXT.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_WHITE);
            colors[WaveformColors.MARKER.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GRAY);
            colors[WaveformColors.MARKER_TEXT.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_WHITE);
            colors[WaveformColors.REL_ARROW.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_MAGENTA);
            colors[WaveformColors.REL_ARROW_HIGHLITE.ordinal()] = SWTResourceManager.getColor(255, 128, 255);
        }
    }

    public void setHighliteRelation(RelationType relationType){
    	if(arrowPainter!=null){
    		boolean redraw = arrowPainter.getHighlightType()!=relationType; 
    		arrowPainter.setHighlightType(relationType);
    		if(redraw) redraw();
    	}
    }
    
    public Point getOrigin() {
        return origin;
    }

    public void setOrigin(Point origin) {
        setOrigin(origin.x, origin.y);
    }

    public void setOrigin(int x, int y) {
        checkWidget();
        ScrollBar hBar = getHorizontalBar();
        hBar.setSelection(-x);
        x = -hBar.getSelection();
        ScrollBar vBar = getVerticalBar();
        vBar.setSelection(-y);
        y = -vBar.getSelection();
        origin.x = x;
        origin.y = y;
        syncScrollBars();
    }

    public long getMaxTime() {
        return maxTime;
    }

    public void setMaxTime(long maxTime) {
        this.maxTime = maxTime;
        syncScrollBars();
    }

    public int getTrackHeight() {
        return trackHeight;
    }

    public void setTrackHeight(int trackHeight) {
        this.trackHeight = trackHeight;
        syncScrollBars();
    }

    public int getZoomLevel() {
        return level;
    }
    
    public int getMaxZoomLevel(){
    	return unitMultiplier.length*unitString.length-1;
    }

    public void setZoomLevel(int level) {
    	long oldScaleFactor=scaleFactor;
    	if(level<unitMultiplier.length*unitString.length){
    		this.level = level;
    		this.scaleFactor = (long) Math.pow(10, level/2);
    		if(level%2==1) this.scaleFactor*=3;
    		ITx tx = arrowPainter.getTx();
    		arrowPainter.setTx(null);
    		/*
    		 * xc = tc/oldScaleFactor
    		 * xoffs = xc+origin.x
    		 * xcn = tc/newScaleFactor
    		 * t0n = (xcn-xoffs)*scaleFactor
    		 */
    		long tc=cursorPainters.get(0).getTime(); // cursor time
    		long xc=tc/oldScaleFactor; // cursor total x-offset
    		long xoffs=xc+origin.x; // cursor offset relative to left border
    		long xcn=tc/scaleFactor; // new total x-offset
    		long originX=xcn-xoffs;
    		if(originX>0)
    			origin.x=(int) -originX; // new cursor time offset relative to left border
    		else
    			origin.x=0;
    		syncScrollBars();
    		arrowPainter.setTx(tx);    		
    		redraw();
    	}
    }

    public long getScaleFactor() {
        return scaleFactor;
    }

    public long getScaleFactorPow10() {
    	int scale = level/unitMultiplier.length;
    	double res = Math.pow(1000, scale);
    	return (long) res;
    }

    public String getUnitStr(){
        return unitString[level/unitMultiplier.length];
    }
     
    public int getUnitMultiplier(){
        return unitMultiplier[level%unitMultiplier.length];
    }
    
    public long getTimeForOffset(int xOffset){
        return (xOffset-origin.x) * scaleFactor;
    }
    
    public void addPainter(IPainter painter) {
        painterList.add(painter);
        redraw();
    }

    public void removePainter(IPainter painter) {
        painterList.remove(painter);
        redraw();
    }

    public void clearAllWaveformPainter() {
        trackAreaPainter.getTrackVerticalOffset().clear();
        wave2painterMap.clear();
        syncScrollBars();
    }

    public void addWaveformPainter(IWaveformPainter painter) {
        trackAreaPainter.addTrackPainter(painter);
        wave2painterMap.put(painter.getTrackEntry().waveform, painter);
        syncScrollBars();
    }

    public List<CursorPainter> getCursorPainters() {
		return cursorPainters;
	}

	/**
     * Dispose the garbage here
     */
    public void dispose() {
        transform.dispose();
        for (WaveformColors c : WaveformColors.values())
            colors[c.ordinal()].dispose();
        super.dispose();
    }

    /* Initalize the scrollbar and register listeners. */
    private void initScrollBars() {
        ScrollBar horizontal = getHorizontalBar();
        horizontal.setEnabled(false);
        horizontal.setVisible(true);
        horizontal.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent event) {
                if (painterList.size() == 0)
                    return;
                setOrigin(-((ScrollBar) event.widget).getSelection(), origin.y);
            }
        });
        ScrollBar vertical = getVerticalBar();
        vertical.setEnabled(false);
        vertical.setVisible(true);
        vertical.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent event) {
                if (painterList.size() == 0)
                    return;
                setOrigin(origin.x, -((ScrollBar) event.widget).getSelection());
            }
        });
    }

    /**
     * Synchronize the scrollbar with the image. If the transform is out of
     * range, it will correct it. This function considers only following factors
     * :<b> transform, image size, client area</b>.
     */
    public void syncScrollBars() {
        if (painterList.size() == 0) {
            redraw();
            return;
        }
        int height = trackAreaPainter.getHeight();
        int width = (int) (maxTime / scaleFactor);
        ScrollBar horizontal = getHorizontalBar();
        horizontal.setIncrement((int) (getClientArea().width / 100));
        horizontal.setPageIncrement(getClientArea().width);
        int cw = getClientArea().width;
        if (width > cw) { /* image is wider than client area */
            horizontal.setMaximum(width);
            horizontal.setEnabled(true);
            if (((int) -origin.x) > horizontal.getMaximum() - cw)
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
        if (height > ch) { /* image is higher than client area */
            vertical.setMaximum(height);
            vertical.setEnabled(true);
            if (((int) -origin.y) > vertical.getMaximum() - ch)
                origin.y = -vertical.getMaximum() + ch;
        } else { /* image is less higher than client area */
            vertical.setMaximum((int) (ch));
            vertical.setEnabled(false);
        }
        vertical.setSelection(-origin.y);
        vertical.setThumb(ch);
        redraw();
        fireSelectionEvent();

    }

    /* Paint function */
    private void paint(GC gc) {
        Rectangle clientRect = getClientArea(); /* Canvas' painting area */
        clientRect.x = -origin.x;
        clientRect.y = -origin.y;
        // reset the transform
        transform.identity();
        // shift the content
        transform.translate(origin.x, origin.y);
        gc.setTransform(transform);
        gc.setClipping(clientRect);
        if (painterList.size() > 0 ) {
            for (IPainter painter : painterList)
                painter.paintArea(gc, clientRect);
        } else {
            gc.fillRectangle(clientRect);
            initScrollBars();
        }
    }

    public List<Object> getClicked(Point point) {
    	LinkedList<Object> result=new LinkedList<>();
        for (IPainter p : Lists.reverse(painterList)) {
            if (p instanceof TrackAreaPainter) {
                int y = point.y - origin.y;
                int x = point.x - origin.x;
                Entry<Integer, IWaveformPainter> entry = trackAreaPainter.getTrackVerticalOffset().floorEntry(y);
                if (entry != null) {
                    if (entry.getValue() instanceof StreamPainter) {
                    	ITx tx = ((StreamPainter) entry.getValue()).getClicked(new Point(x, y - entry.getKey()));
                    	if(tx!=null)
                    		result.add(tx);
                    } 
                    result.add(entry.getValue().getTrackEntry());
                }
            } else if (p instanceof CursorPainter) {
                if (Math.abs(point.x - origin.x - ((CursorPainter) p).getTime()/scaleFactor) < 2) {
                	result.add(p);
                }
            }
        }
        return result;
    }

    public List<Object> getEntriesAtPosition(IWaveform<? extends IWaveformEvent> iWaveform, int i) {
    	LinkedList<Object> result=new LinkedList<>();
        int x = i - origin.x;
        for(IPainter p: wave2painterMap.values()){
        	if (p instanceof StreamPainter && ((StreamPainter)p).getStream()==iWaveform) {
        		result.add(((StreamPainter) p).getClicked(new Point(x, trackHeight/2)));
        	}
        }
        return result;
    }

    public void setSelected(ITx currentSelection) {
        this.currentSelection = currentSelection;
        if (currentSelection != null)
            reveal(currentSelection);
        arrowPainter.setTx(currentSelection);
        redraw();
    }

    public void reveal(ITx tx) {
        int lower = (int) (tx.getBeginTime() / scaleFactor);
        int higher = (int) (tx.getEndTime() / scaleFactor);
        Point size = getSize();
        size.x -= getVerticalBar().getSize().x + 2;
        size.y -= getHorizontalBar().getSize().y;
        if (lower < -origin.x) {
            setOrigin(-lower, origin.y);
        } else if (higher > (size.x - origin.x)) {
            setOrigin(size.x - higher, origin.y);
        }
        for (IWaveformPainter painter : wave2painterMap.values()) {
            if (painter instanceof StreamPainter && ((StreamPainter) painter).getStream() == tx.getStream()) {
                int top = painter.getVerticalOffset() + trackHeight * tx.getConcurrencyIndex();
                int bottom = top + trackHeight;
                if (top < -origin.y) {
                    setOrigin(origin.x, -(top-trackHeight));
                } else if (bottom > (size.y - origin.y)) {
                    setOrigin(origin.x, size.y - bottom);
                }
            }
        }
    }
    
    public void reveal(long time) {
        int scaledTime = (int) (time / scaleFactor);
        Point size = getSize();
        size.x -= getVerticalBar().getSize().x + 2;
        size.y -= getHorizontalBar().getSize().y;
        if (scaledTime < -origin.x) {
            setOrigin(-scaledTime+10, origin.y);
        } else if (scaledTime > (size.x - origin.x)) {
            setOrigin(size.x - scaledTime-30, origin.y);
        }
    }

    public int getRulerHeight() {
        return rulerHeight;
    }

    public void setRulerHeight(int rulerHeight) {
        this.rulerHeight = rulerHeight;
    }

    public void addSelectionListener(SelectionAdapter selectionAdapter) {
        selectionListeners.add(selectionAdapter);
    }

    public void removeSelectionListener(SelectionAdapter selectionAdapter) {
        selectionListeners.remove(selectionAdapter);
    }

    /**
     * 
     */
    protected void fireSelectionEvent() {
        Event e = new Event();
        e.widget = this;
        e.detail=SWT.SELECTED;
        e.type=SWT.Selection;
        SelectionEvent ev = new SelectionEvent(e);
        ev.x = origin.x;
        ev.y = origin.y;
        for (SelectionAdapter a : selectionListeners) {
            a.widgetSelected(ev);
        }
    }

}
