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
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.wb.swt.SWTResourceManager;

import com.google.common.collect.Lists;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;

public class WaveformCanvas extends Canvas {
    public enum Colors {
        LINE, LINE_HIGHLITE, 
        TRACK_BG_EVEN, TRACK_BG_ODD, TRACK_BG_HIGHLITE, 
        TX_BG, TX_BG_HIGHLITE, TX_BORDER,
        SIGNAL0, SIGNAL1, SIGNALZ, SIGNALX, SIGNAL_TEXT, 
        CURSOR, CURSOR_DRAG, CURSOR_TEXT
    }

    Color[] colors = new Color[Colors.values().length];

    private int trackHeight = 50;
    private long scaleFactor = 1000000L;
    String unit="ns";
    private int level = 6;
    private final static String[] unitString={"fs", "ps", "ns", "µs", "ms", "s"};
    private final static int[] unitMultiplier={1, 10, 100};
    private long maxTime;
    protected Point origin; /* original size */
    protected Transform transform;
    protected int rulerHeight=40;
    protected List<IPainter> painterList;
    TreeMap<Integer, IWaveformPainter> trackVerticalOffset;

    protected List<IWaveform<? extends IWaveformEvent>> streams;

    ITx currentSelection;
    IWaveform<? extends IWaveformEvent> currentWaveformSelection;

    private List<SelectionAdapter> selectionListeners;

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
        trackVerticalOffset = new TreeMap<Integer, IWaveformPainter>();
        selectionListeners = new LinkedList<>();
        initScrollBars();
        initColors(null);
    }

    private void initColors(HashMap<Colors, RGB> colourMap) {
        Display d = getDisplay();
        if (colourMap != null) {
            for (Colors c : Colors.values()) {
                if (colourMap.containsKey(c)) {
                    colors[c.ordinal()].dispose();
                    colors[c.ordinal()] = new Color(d, colourMap.get(c));
                }
            }
        } else {
            colors[Colors.LINE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
            colors[Colors.LINE_HIGHLITE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_CYAN);
            colors[Colors.TRACK_BG_EVEN.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_BLACK);
            colors[Colors.TRACK_BG_ODD.ordinal()] = SWTResourceManager.getColor(40, 40, 40);
            colors[Colors.TRACK_BG_HIGHLITE.ordinal()] = SWTResourceManager.getColor(40, 40, 80);
            colors[Colors.TX_BG.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GREEN);
            colors[Colors.TX_BG_HIGHLITE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
            colors[Colors.TX_BORDER.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
            colors[Colors.SIGNAL0.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
            colors[Colors.SIGNAL1.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
            colors[Colors.SIGNALZ.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GRAY);
            colors[Colors.SIGNALX.ordinal()] = SWTResourceManager.getColor(255,  128,  182);
            colors[Colors.SIGNAL_TEXT.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_WHITE);
            colors[Colors.CURSOR.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
            colors[Colors.CURSOR_DRAG.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GRAY);
            colors[Colors.CURSOR_TEXT.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_WHITE);
        }
    }

    public List<IWaveform<? extends IWaveformEvent>> getStreams() {
        return streams;
    }

    public void setStreams(List<IWaveform<? extends IWaveformEvent>> streams) {
        this.streams = streams;
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

    public void setZoomLevel(int level) {
        this.level = level;
        this.scaleFactor = (long) Math.pow(10, level);
        syncScrollBars();
    }

    public long getScaleFactor() {
        return scaleFactor;
    }

    public String getUnitStr(){
        return unitString[level/3];
    }
    
    public int getUnitMultiplier(){
        return unitMultiplier[level%3];
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

    public void clearAllWavefromPainter() {
        trackVerticalOffset.clear();
        syncScrollBars();
    }

    public void addWavefromPainter(int yoffs, IWaveformPainter painter) {
        trackVerticalOffset.put(yoffs+rulerHeight, painter);
        syncScrollBars();
    }

    /**
     * Dispose the garbage here
     */
    public void dispose() {
        transform.dispose();
        for (Colors c : Colors.values())
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
    private void syncScrollBars() {
        if (painterList.size() == 0) {
            redraw();
            return;
        }
        int height = 1;
        if (trackVerticalOffset.size() > 0)
            height = trackVerticalOffset.lastKey() + trackVerticalOffset.lastEntry().getValue().getMinHeight();

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
        if (painterList.size() > 0 && trackVerticalOffset.size() > 0) {
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
            if (p instanceof TrackPainter) {
                int y = point.y - origin.y;
                int x = point.x - origin.x;
                Entry<Integer, IWaveformPainter> entry = trackVerticalOffset.floorEntry(y);
                if (entry != null) {
                    if (entry.getValue() instanceof StreamPainter) {
                    	result.add(((StreamPainter) entry.getValue()).getClicked(new Point(x, y - entry.getKey())));
                    } else if (entry.getValue() instanceof SignalPainter)
                    	result.add(((SignalPainter) entry.getValue()).getSignal());
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
        for(IPainter p: trackVerticalOffset.values()){
        	if (p instanceof StreamPainter && ((StreamPainter)p).getStream()==iWaveform) {
        		result.add(((StreamPainter) p).getClicked(new Point(x, trackHeight/2)));
        	}
        }
        return result;
    }

    public void setSelected(ITx currentSelection, IWaveform<? extends IWaveformEvent> currentWaveformSelection) {
        this.currentSelection = currentSelection;
        this.currentWaveformSelection = currentWaveformSelection;
        if (currentSelection != null)
            reveal(currentSelection);
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
        for (Entry<Integer, IWaveformPainter> entry : trackVerticalOffset.entrySet()) {
            if (entry.getValue() instanceof StreamPainter && ((StreamPainter) entry.getValue()).getStream() == tx.getStream()) {
                int top = entry.getKey() + trackHeight * tx.getConcurrencyIndex();
                int bottom = top + trackHeight;
                if (top < -origin.y) {
                    setOrigin(origin.x, -top);
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
