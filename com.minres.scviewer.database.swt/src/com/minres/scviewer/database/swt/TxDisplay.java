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
package com.minres.scviewer.database.swt;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.core.runtime.ListenerList;
import org.eclipse.jface.util.LocalSelectionTransfer;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CLabel;
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.custom.ScrolledComposite;
import org.eclipse.swt.dnd.DND;
import org.eclipse.swt.dnd.DragSource;
import org.eclipse.swt.dnd.DragSourceEvent;
import org.eclipse.swt.dnd.DragSourceListener;
import org.eclipse.swt.dnd.DropTarget;
import org.eclipse.swt.dnd.DropTargetEvent;
import org.eclipse.swt.dnd.DropTargetListener;
import org.eclipse.swt.dnd.TextTransfer;
import org.eclipse.swt.dnd.Transfer;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.graphics.TextLayout;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.wb.swt.SWTResourceManager;

import swing2swt.layout.BorderLayout;

import com.google.common.collect.Lists;
import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.swt.internal.CursorPainter;
import com.minres.scviewer.database.swt.internal.IWaveformPainter;
import com.minres.scviewer.database.swt.internal.ObservableList;
import com.minres.scviewer.database.swt.internal.Ruler;
import com.minres.scviewer.database.swt.internal.SignalPainter;
import com.minres.scviewer.database.swt.internal.StreamPainter;
import com.minres.scviewer.database.swt.internal.TrackPainter;
import com.minres.scviewer.database.swt.internal.WaveformCanvas;

public class TxDisplay implements PropertyChangeListener, ISelectionProvider, MouseListener {
    private ListenerList listeners = new ListenerList();

    private static final String SELECTION = "selection";
    private ITx currentSelection;
    private IWaveform<? extends IWaveformEvent> currentWaveformSelection;

    private ScrolledComposite nameListScrolled;
    private ScrolledComposite valueListScrolled;

    private Canvas nameList;
    private Canvas valueList;
    WaveformCanvas trackList;

    private Composite top;

    protected ObservableList<IWaveform<? extends IWaveformEvent>> streams;
    Vector<CursorPainter> cursorPainters;

    private Composite trackPane;
    private Ruler ruler;
    private int trackVerticalHeight;
    private TreeMap<Integer, IWaveform<? extends IWaveformEvent>> trackVerticalOffset;
    private HashMap<IWaveform<? extends IWaveformEvent>, String> actualValues;
    // private long maxTime=0;
    private Font nameFont, nameFontB;

    public TxDisplay(Composite parent) {
        trackVerticalOffset = new TreeMap<Integer, IWaveform<? extends IWaveformEvent>>();
        trackVerticalHeight=0;
        actualValues = new HashMap<IWaveform<? extends IWaveformEvent>, String>();
        cursorPainters = new Vector<CursorPainter>();

        nameFont = parent.getDisplay().getSystemFont();
        nameFontB = SWTResourceManager.getBoldFont(nameFont);

        streams = new ObservableList<IWaveform<? extends IWaveformEvent>>();
        streams.addPropertyChangeListener(this);

        top = new Composite(parent, SWT.NONE);
        top.setLayout(new FillLayout(SWT.HORIZONTAL));

        SashForm topSash = new SashForm(top, SWT.SMOOTH);
        topSash.setBackground(topSash.getDisplay().getSystemColor(SWT.COLOR_GRAY));

        Composite composite = new Composite(topSash, SWT.NONE);
        composite.setLayout(new FillLayout(SWT.HORIZONTAL));

        SashForm leftSash = new SashForm(composite, SWT.SMOOTH);
        leftSash.setBackground(leftSash.getDisplay().getSystemColor(SWT.COLOR_GRAY));

        Composite namePane = createTextPane(leftSash, "Name");
        namePane.setBackground(namePane.getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND));

        nameListScrolled = new ScrolledComposite(namePane, SWT.H_SCROLL | SWT.V_SCROLL);
        nameListScrolled.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));
        nameListScrolled.setExpandHorizontal(true);
        nameListScrolled.setExpandVertical(true);
        nameListScrolled.setAlwaysShowScrollBars(true);
        nameListScrolled.addControlListener(new ControlAdapter() {
            @Override
            public void controlResized(ControlEvent e) {
                nameListScrolled.getVerticalBar().setVisible(false);

            }
        });
        nameList = new Canvas(nameListScrolled, SWT.NONE) {
            @Override
            public Point computeSize(int wHint, int hHint, boolean changed) {
                Rectangle bounds = super.getClientArea();
                return new Point(bounds.width, bounds.height);
            }
        };
        nameList.addListener(SWT.Paint, new Listener() {
            @Override
            public void handleEvent(Event event) {
                GC gc = event.gc;
                Rectangle rect = ((Canvas) event.widget).getClientArea();
                paintNames(gc, rect);
            }
        });
        nameList.addMouseListener(this);
        nameListScrolled.setContent(nameList);

        Composite valuePane = createTextPane(leftSash, "Value");
        valuePane.setBackground(valuePane.getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND));
        valueListScrolled = new ScrolledComposite(valuePane, SWT.H_SCROLL | SWT.V_SCROLL);
        valueListScrolled.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));
        valueListScrolled.setExpandHorizontal(true);
        valueListScrolled.setExpandVertical(true);
        valueListScrolled.setAlwaysShowScrollBars(true);
        valueListScrolled.addControlListener(new ControlAdapter() {
            @Override
            public void controlResized(ControlEvent e) {
                valueListScrolled.getVerticalBar().setVisible(false);

            }
        });
        valueList = new Canvas(valueListScrolled, SWT.NONE) {
            @Override
            public Point computeSize(int wHint, int hHint, boolean changed) {
                Rectangle bounds = super.getClientArea();
                return new Point(bounds.width, bounds.height);
            }
        };
        valueList.addListener(SWT.Paint, new Listener() {
            @Override
            public void handleEvent(Event event) {
                GC gc = event.gc;
                Rectangle rect = ((Canvas) event.widget).getClientArea();
                paintValues(gc, rect);
            }
        });
        valueList.addMouseListener(this);
        valueListScrolled.setContent(valueList);

        trackPane = new Composite(topSash, SWT.NONE);
        trackPane.setLayout(new BorderLayout(0, 0));
        ruler = new Ruler(trackPane, SWT.NONE);
        ruler.setLayoutData(BorderLayout.NORTH);

        trackList = new WaveformCanvas(trackPane, SWT.NONE);
        trackList.setLayoutData(BorderLayout.CENTER);
        trackList.setStreams(streams);
        trackList.setRuler(ruler);
        trackList.addPainter(new TrackPainter(trackList));
        CursorPainter cp = new CursorPainter(trackList, trackList.getScaleFactor() * 10);
        trackList.addPainter(cp);
        cursorPainters.add(cp);
        trackList.setMaxTime(1);
        trackList.addMouseListener(this);

        nameListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                int y = ((ScrollBar) e.widget).getSelection();
                Point v = valueListScrolled.getOrigin();
                valueListScrolled.setOrigin(v.x, y);
                Point t = trackList.getOrigin();
                trackList.setOrigin(t.x, -y);
            }
        });
        valueListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                int y = ((ScrollBar) e.widget).getSelection();
                nameListScrolled.setOrigin(nameListScrolled.getOrigin().x, y);
                trackList.setOrigin(trackList.getOrigin().x, -y);
            }
        });
        trackList.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                int y = trackList.getVerticalBar().getSelection();
                nameListScrolled.setOrigin(nameListScrolled.getOrigin().x, y);
                valueListScrolled.setOrigin(valueListScrolled.getOrigin().x, y);
            }
        });
        topSash.setWeights(new int[] { 30, 70 });
        leftSash.setWeights(new int[] { 75, 25 });

        createDragSource(nameList);
        createDragSource(valueList);
        createDropTarget(nameList);
        createDropTarget(valueList);
    }

    private Composite createTextPane(SashForm leftSash, String text) {
        Composite namePane = new Composite(leftSash, SWT.NONE);
        GridLayout gl_namePane = new GridLayout(1, false);
        gl_namePane.verticalSpacing = 0;
        gl_namePane.marginWidth = 0;
        gl_namePane.horizontalSpacing = 0;
        gl_namePane.marginHeight = 0;
        namePane.setLayout(gl_namePane);

        CLabel nameLabel = new CLabel(namePane, SWT.NONE);
        GridData gd_nameLabel = new GridData(SWT.CENTER, SWT.CENTER, true, false, 1, 1);
        gd_nameLabel.heightHint = Ruler.height - 2;
        nameLabel.setLayoutData(gd_nameLabel);
        nameLabel.setText(text);

        Label nameSep = new Label(namePane, SWT.SEPARATOR | SWT.HORIZONTAL);
        nameSep.setBackground(SWTResourceManager.getColor(SWT.COLOR_DARK_GRAY));
        nameSep.setForeground(SWTResourceManager.getColor(SWT.COLOR_BLACK));
        GridData gd_nameSep = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
        gd_nameSep.heightHint = 2;
        nameSep.setLayoutData(gd_nameSep);
        return namePane;
    }

    @Override
    public void propertyChange(PropertyChangeEvent pce) {
        if ("size".equals(pce.getPropertyName()) || "content".equals(pce.getPropertyName())) {
            updateTracklist();
        }
    }

    @SuppressWarnings("unchecked")
    protected void updateTracklist() {
        int trackVerticalHeight = 0;
        int nameMaxWidth = 0;
        int previousHeight = trackVerticalOffset.size() == 0 ? 0 : trackVerticalOffset.lastKey();
        IWaveformPainter painter = null;
        trackVerticalOffset.clear();
        actualValues.clear();
        trackList.clearAllWavefromPainter();
        boolean even = true;
        TextLayout tl = new TextLayout(trackList.getDisplay());
        tl.setFont(nameFontB);
        for (IWaveform<? extends IWaveformEvent> waveform : streams) {
            int height = trackList.getTrackHeight();
            if (waveform instanceof ITxStream<?>) {
                height *= ((ITxStream<? extends ITxEvent>) waveform).getMaxConcurrency();
                painter = new StreamPainter(trackList, even, height, (ITxStream<? extends ITxEvent>) waveform);
                actualValues.put(waveform, "");
            } else if (waveform instanceof ISignal<?>) {
                painter = new SignalPainter(trackList, even, height, (ISignal<?>) waveform);
                actualValues.put(waveform, "---");
            }
            trackList.addWavefromPainter(trackVerticalHeight, painter);
            trackVerticalOffset.put(trackVerticalHeight, waveform);
            tl.setText(waveform.getFullName());
            nameMaxWidth = Math.max(nameMaxWidth, tl.getBounds().width);
            trackVerticalHeight += height;
            even = !even;
        }
        nameList.setSize(nameMaxWidth + 15, trackVerticalHeight);
        nameListScrolled.setMinSize(nameMaxWidth + 15, trackVerticalHeight);
        valueList.setSize(calculateValueWidth(), trackVerticalHeight);
        valueListScrolled.setMinSize(calculateValueWidth(), trackVerticalHeight);
        nameList.redraw();
        valueList.redraw();
        trackList.redraw();
        top.layout(new Control[] { valueList, nameList, trackList });
        if (previousHeight > trackVerticalOffset.lastKey()) {
            Point o = trackList.getOrigin();
            trackList.setOrigin(o.x, o.y - (previousHeight - trackVerticalOffset.lastKey()));
        }
        setSelection(new StructuredSelection());
    }

    private int calculateValueWidth() {
        TextLayout tl = new TextLayout(trackList.getDisplay());
        tl.setFont(nameFontB);
        int valueMaxWidth = 0;
        for (String v : actualValues.values()) {
            tl.setText(v);
            valueMaxWidth = Math.max(valueMaxWidth, tl.getBounds().width);
        }
        return valueMaxWidth + 15;
    }

    @Override
    public void addSelectionChangedListener(ISelectionChangedListener listener) {
        listeners.add(listener);
    }

    @Override
    public void removeSelectionChangedListener(ISelectionChangedListener listener) {
        listeners.remove(listener);
    }

    public Control getControl() {
        return top;
    }

    @Override
    public ISelection getSelection() {
        if (currentSelection != null)
            return new StructuredSelection(currentSelection);
        else if (currentWaveformSelection != null)
            return new StructuredSelection(currentWaveformSelection);
        else
            return null;
    }

    @SuppressWarnings("unchecked")
    @Override
    public void setSelection(ISelection selection) {
        boolean selectionChanged = false;
        if (selection instanceof IStructuredSelection) {
            if(((IStructuredSelection) selection).size()==0){
                selectionChanged = currentSelection!=null||currentWaveformSelection!=null;                
                currentSelection = null;
                currentWaveformSelection = null;
            } else {
                Object sel = ((IStructuredSelection) selection).getFirstElement();
                if (sel instanceof ITx && currentSelection != sel) {
                    currentSelection = (ITx) sel;
                    currentWaveformSelection = currentSelection.getStream();
                    selectionChanged = true;
                } else if (sel instanceof IWaveform<?> && currentWaveformSelection != sel) {
                    currentSelection = null;
                    currentWaveformSelection = (IWaveform<? extends IWaveformEvent>) sel;
                    selectionChanged = true;
                }
            }
        } else {
            if (currentSelection != null || currentWaveformSelection != null)
                selectionChanged = true;
            currentSelection = null;
            currentWaveformSelection = null;
        }
        if (currentWaveformSelection != null && !streams.contains(currentWaveformSelection)) {
            streams.add(currentWaveformSelection);
        }
        if (selectionChanged) {
            trackList.setSelected(currentSelection, currentWaveformSelection);
            nameList.setData(SELECTION, currentWaveformSelection);
            valueList.redraw();
            nameList.redraw();
            Object[] list = listeners.getListeners();
            for (int i = 0; i < list.length; i++) {
                ((ISelectionChangedListener) list[i]).selectionChanged(new SelectionChangedEvent(this, selection));
            }
        }
    }

    @SuppressWarnings("unchecked")
    public void moveSelection(GotoDirection direction) {
        if (currentWaveformSelection instanceof ITxStream<?>) {
            ITxStream<ITxEvent> stream = (ITxStream<ITxEvent>) currentWaveformSelection;
            ITx transaction = null;
            if (direction == GotoDirection.NEXT) {
                List<ITxEvent> thisEntryList = stream.getEvents().get(currentSelection.getBeginTime());
                boolean meFound=false;
                for (ITxEvent evt : thisEntryList) {
                    if (evt.getType() == ITxEvent.Type.BEGIN) {
                        if(meFound){
                            transaction = evt.getTransaction();
                            break;
                        }
                        meFound|= evt.getTransaction().getId()==currentSelection.getId();
                    }
                }
                if (transaction == null){
                    Entry<Long, List<ITxEvent>> entry = stream.getEvents().higherEntry(currentSelection.getBeginTime());
                    if (entry != null) do {
                        for (ITxEvent evt : entry.getValue()) {
                            if (evt.getType() == ITxEvent.Type.BEGIN) {
                                transaction = evt.getTransaction();
                                break;
                            }
                        }
                        if (transaction == null)
                            entry = stream.getEvents().higherEntry(entry.getKey());
                    } while (entry != null && transaction == null);
                }
            } else if (direction == GotoDirection.PREV) {
                List<ITxEvent> thisEntryList = stream.getEvents().get(currentSelection.getBeginTime());
                boolean meFound=false;
                for (ITxEvent evt :  Lists.reverse(thisEntryList)) {
                    if (evt.getType() == ITxEvent.Type.BEGIN) {
                        if(meFound){
                            transaction = evt.getTransaction();
                            break;
                        }
                        meFound|= evt.getTransaction().getId()==currentSelection.getId();
                    }
                }
                if (transaction == null){
                    Entry<Long, List<ITxEvent>> entry = stream.getEvents().lowerEntry(currentSelection.getBeginTime());
                    if (entry != null)
                        do {
                            for (ITxEvent evt : Lists.reverse(entry.getValue())) {
                                if (evt.getType() == ITxEvent.Type.BEGIN) {
                                    transaction = evt.getTransaction();
                                    break;
                                }
                            }
                            if (transaction == null)
                                entry = stream.getEvents().lowerEntry(entry.getKey());
                        } while (entry != null && transaction == null);
                }
            }
            if (transaction != null) {
                setSelection(new StructuredSelection(transaction));
            }
        }
    }

    @Override
    public void mouseDoubleClick(MouseEvent e) {
    }

    @Override
    public void mouseDown(MouseEvent e) {
        if (e.button == 1 || e.button == 3) {
            if (e.widget == trackList) {
                Object o = trackList.getClicked(new Point(e.x, e.y));
                if (o != null)
                    setSelection(new StructuredSelection(o));
            } else if (e.widget == valueList || e.widget == nameList) {
                Entry<Integer, IWaveform<? extends IWaveformEvent>> entry = trackVerticalOffset.floorEntry(e.y);
                if (entry != null)
                    setSelection(new StructuredSelection(entry.getValue()));
            }
        }
        if (e.button == 3) {
            top.getMenu().setVisible(true);
        }
    }

    @Override
    public void mouseUp(MouseEvent e) {
    }

    public List<IWaveform<? extends IWaveformEvent>> getStreamList() {
        return streams;
    }

    protected void paintNames(GC gc, Rectangle rect) {
        if (streams.size() > 0) {
            @SuppressWarnings("unchecked")
            IWaveform<? extends IWaveformEvent> wave = (IWaveform<? extends IWaveformEvent>) nameList.getData(SELECTION);
            Integer firstKey = trackVerticalOffset.floorKey(rect.y);
            if (firstKey == null)
                firstKey = trackVerticalOffset.firstKey();
            Integer lastKey = trackVerticalOffset.floorKey(rect.y + rect.height);
            Rectangle subArea = new Rectangle(rect.x, 0, rect.width, trackList.getTrackHeight());
            if (lastKey == firstKey) {
                IWaveform<? extends IWaveformEvent> w = trackVerticalOffset.get(firstKey);
                if (w instanceof ITxStream<?>)
                    subArea.height *= ((ITxStream<?>) w).getMaxConcurrency();
                drawTextFormat(gc, subArea, firstKey, w.getFullName(), wave != null && w.getId() == wave.getId());
            } else {
                for (Entry<Integer, IWaveform<? extends IWaveformEvent>> entry : trackVerticalOffset.subMap(firstKey, true, lastKey, true)
                        .entrySet()) {
                    IWaveform<? extends IWaveformEvent> w = entry.getValue();
                    subArea.height = trackList.getTrackHeight();
                    if (w instanceof ITxStream<?>)
                        subArea.height *= ((ITxStream<?>) w).getMaxConcurrency();
                    drawTextFormat(gc, subArea, entry.getKey(), w.getFullName(), wave != null && w.getId() == wave.getId());
                }
            }
        }
    }

    @SuppressWarnings("unchecked")
    protected void paintValues(GC gc, Rectangle rect) {
        if (streams.size() > 0) {
            IWaveform<? extends IWaveformEvent> wave = (IWaveform<? extends IWaveformEvent>) nameList.getData(SELECTION);
            Integer firstKey = trackVerticalOffset.floorKey(rect.y);
            if (firstKey == null)
                firstKey = trackVerticalOffset.firstKey();
            Integer lastKey = trackVerticalOffset.floorKey(rect.y + rect.height);
            Rectangle subArea = new Rectangle(rect.x, 0, rect.width, trackList.getTrackHeight());
            if (lastKey == firstKey) {
                IWaveform<? extends IWaveformEvent> w = trackVerticalOffset.get(firstKey);
                if (w instanceof ITxStream<?>)
                    subArea.height *= ((ITxStream<?>) w).getMaxConcurrency();
                drawTextFormat(gc, subArea, firstKey, actualValues.get(w), wave != null && w.getId() == wave.getId());
            } else {
                for (Entry<Integer, IWaveform<? extends IWaveformEvent>> entry : trackVerticalOffset.subMap(firstKey, true, lastKey, true)
                        .entrySet()) {
                    IWaveform<? extends IWaveformEvent> w = entry.getValue();
                    subArea.height = trackList.getTrackHeight();
                    if (w instanceof ITxStream<?>)
                        subArea.height *= ((ITxStream<?>) w).getMaxConcurrency();
                    drawTextFormat(gc, subArea, entry.getKey(), actualValues.get(w), wave != null && w.getId() == wave.getId());
                }
            }
        }
    }

    protected void drawTextFormat(GC gc, Rectangle subArea, int yOffset, String value, boolean highlite) {
        Point size = gc.textExtent(value);
        if (highlite) {
            gc.setBackground(SWTResourceManager.getColor(SWT.COLOR_LIST_SELECTION));
            gc.setForeground(SWTResourceManager.getColor(SWT.COLOR_LIST_SELECTION_TEXT));
            gc.fillRectangle(subArea.x, subArea.y + yOffset, subArea.width, subArea.height);
            gc.setFont(nameFontB);
        } else {
            gc.setBackground(SWTResourceManager.getColor(SWT.COLOR_LIST_BACKGROUND));
            gc.setForeground(SWTResourceManager.getColor(SWT.COLOR_LIST_FOREGROUND));
            gc.setFont(nameFont);
        }
        gc.drawText(value, subArea.x + 5, subArea.y + yOffset + (trackList.getTrackHeight() - size.y) / 2, true);
    }

    public long getMaxTime() {
        return trackList.getMaxTime();
    }

    public void setMaxTime(long maxTime) {
        this.trackList.setMaxTime(maxTime);
    }

    public void setZoomLevel(int scale) {
        trackList.setZoomLevel(scale);
    }

    public int getZoomLevel() {
        return trackList.getZoomLevel();
    }

    private void createDragSource(final Canvas sourceText) {
        Transfer[] types = new Transfer[] { LocalSelectionTransfer.getTransfer() };
        DragSource dragSource = new DragSource(sourceText, DND.DROP_MOVE);
        dragSource.setTransfer(types);
        dragSource.addDragListener(new DragSourceListener() {

            public void dragStart(DragSourceEvent event) {
                if (event.y < trackVerticalHeight) {
                    // event.data =
                    // trackVerticalOffset.floorEntry(event.y).getValue().getFullName();
                    event.doit = true;
                    LocalSelectionTransfer.getTransfer().setSelection(getSelection());
                    // System.out.println("dragStart at location "+new
                    // Point(event.x, event.y));
                }
            }

            public void dragSetData(DragSourceEvent event) {
                event.data =getSelection();
                // System.out.println("dragSetData with data " + event.data);
            }

            public void dragFinished(DragSourceEvent event) {
                // do nothing
            }
        });
    }

    private void createDropTarget(final Canvas targetText) {
        Transfer[] types = new Transfer[] { LocalSelectionTransfer.getTransfer() };
        DropTarget dropTarget = new DropTarget(targetText, DND.DROP_MOVE);
        dropTarget.setTransfer(types);

        dropTarget.addDropListener(new DropTargetListener() {

            public void dragEnter(DropTargetEvent event) {
            }

            public void dragLeave(DropTargetEvent event) {
            }

            public void dragOperationChanged(DropTargetEvent event) {
            }

            public void dragOver(DropTargetEvent event) {
            }

            public void drop(DropTargetEvent event) {
                if (LocalSelectionTransfer.getTransfer().isSupportedType(event.currentDataType)){
                    ISelection sel = LocalSelectionTransfer.getTransfer().getSelection();
                    if(sel!=null && sel instanceof IStructuredSelection){
                        Object source = ((IStructuredSelection)sel).getFirstElement();
                        DropTarget tgt = (DropTarget) event.widget;
                        Point dropPoint = ((Canvas) tgt.getControl()).toControl(event.x, event.y);
                        Object target = trackVerticalOffset.floorEntry(dropPoint.y).getValue();
                        if(source instanceof IWaveform<?> && target instanceof IWaveform<?>){
//                            int srcIdx=streams.indexOf(source);
//                            int tgtIdx=streams.indexOf(target);
//                            if(srcIdx<tgtIdx){
//                                streams.rotate(srcIdx,  tgtIdx, -1);
//                            } else {
//                                streams.rotate(tgtIdx,  srcIdx+1, 1);
//                            }
                            streams.remove(source);
                            int tgtIdx=streams.indexOf(target);
                            streams.add(tgtIdx, (IWaveform<? extends IWaveformEvent>) source);
                        }
                    }
                }
            }

            public void dropAccept(DropTargetEvent event) {
                if (event.detail != DND.DROP_MOVE || event.y > trackVerticalOffset.lastKey() + trackList.getTrackHeight()) {
                    event.detail = DND.DROP_NONE;
                }
            }
        });
    }

}
