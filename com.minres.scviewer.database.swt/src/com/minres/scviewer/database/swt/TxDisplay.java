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
package com.minres.scviewer.database.swt;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.NavigableMap;
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
import org.eclipse.swt.dnd.DragSourceAdapter;
import org.eclipse.swt.dnd.DragSourceEvent;
import org.eclipse.swt.dnd.DropTarget;
import org.eclipse.swt.dnd.DropTargetAdapter;
import org.eclipse.swt.dnd.DropTargetEvent;
import org.eclipse.swt.dnd.Transfer;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.MouseAdapter;
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
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.wb.swt.SWTResourceManager;

import com.google.common.collect.Lists;
import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ISignalChange;
import com.minres.scviewer.database.ISignalChangeMulti;
import com.minres.scviewer.database.ISignalChangeSingle;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.swt.internal.CursorPainter;
import com.minres.scviewer.database.swt.internal.IWaveformPainter;
import com.minres.scviewer.database.swt.internal.ObservableList;
import com.minres.scviewer.database.swt.internal.RulerPainter;
import com.minres.scviewer.database.swt.internal.SignalPainter;
import com.minres.scviewer.database.swt.internal.StreamPainter;
import com.minres.scviewer.database.swt.internal.TrackPainter;
import com.minres.scviewer.database.swt.internal.WaveformCanvas;

import swing2swt.layout.BorderLayout;

public class TxDisplay implements PropertyChangeListener, ISelectionProvider  {
	
	private ListenerList selectionChangedListeners = new ListenerList();
	
	private PropertyChangeSupport pcs;

	public static final String CURSOR_PROPERTY = "cursor_time";

	public static final String MARKER_PROPERTY = "marker_time";

	private static final String SELECTION = "selection";
	
	private ITx currentTxSelection;
	
	private IWaveform<? extends IWaveformEvent> currentWaveformSelection;

	private ScrolledComposite nameListScrolled;
	
	private ScrolledComposite valueListScrolled;

	private Canvas nameList;
	
	private Canvas valueList;
	
	WaveformCanvas waveformList;

	private Composite top;

	protected ObservableList<IWaveform<? extends IWaveformEvent>> streams;
	
	Vector<CursorPainter> cursorPainters;

	int selectedMarker = 0;
	
	private Composite trackPane;
	
	private int trackVerticalHeight;
	
	private TreeMap<Integer, IWaveform<? extends IWaveformEvent>> trackVerticalOffset;
	
	private HashMap<IWaveform<? extends IWaveformEvent>, String> actualValues;
	
	private Font nameFont, nameFontB;

	protected MouseListener nameValueMouseListener = new MouseAdapter() {
		@Override
		public void mouseDown(MouseEvent e) {
			if ((e.button == 1 || e.button == 3)) {
				Entry<Integer, IWaveform<? extends IWaveformEvent>> entry = trackVerticalOffset.floorEntry(e.y);
				if (entry != null)
					setSelection(new StructuredSelection(entry.getValue()));
			} 
			if (e.button == 3) {
				Menu topMenu= top.getMenu();
				if(topMenu!=null) topMenu.setVisible(true);
			}
		}
	};

	protected MouseListener waveformMouseListener = new MouseAdapter(){
		Point start;
		List<Object> initialSelected;
		
		@Override
		public void mouseDown(MouseEvent e) {
			start=new Point(e.x, e.y);
			if (e.button ==  1) {	
				initialSelected = waveformList.getClicked(start);
			} else if (e.button == 3) {
				List<Object> hitted = waveformList.getClicked(start);
				for(Object entry:hitted){
					if(entry instanceof IWaveform<?>){
						setSelection(new StructuredSelection(entry));
						break;
					}else if(entry instanceof ITx){
						setSelection(new StructuredSelection(((ITx)entry).getStream()));
						break;
					}
				}
				Menu topMenu= top.getMenu();
				if(topMenu!=null) topMenu.setVisible(true);
			}
		}

		@Override
		public void mouseUp(MouseEvent e) {
			if (e.button ==  1) {
				if(Math.abs(e.x-start.x)<3 && Math.abs(e.y-start.y)<3){				
					// first set time
					setCursorTime(snapOffsetToEvent(e));
					// then set selection and reveal
					setSelection(new StructuredSelection(initialSelected));
					e.widget.getDisplay().asyncExec(new Runnable() {
						@Override
						public void run() {
							waveformList.redraw();
							updateValueList();
						}
					});
				}
			}else	if (e.button ==  2) {
				setMarkerTime(snapOffsetToEvent(e), selectedMarker);
				e.widget.getDisplay().asyncExec(new Runnable() {
					@Override
					public void run() {
						waveformList.redraw();
						updateValueList();
					}
				});
			}        
		}

		protected long snapOffsetToEvent(MouseEvent e) {
			long time= waveformList.getTimeForOffset(e.x);
			long scaling=5*waveformList.getScaleFactor();
			for(Object o:waveformList.getClicked(start)){
				Entry<Long, ?> floorEntry=null, ceilEntry=null;
				if(o instanceof ISignal<?>){
					NavigableMap<Long, ?> map = ((ISignal<?>)o).getEvents();
					floorEntry = map.floorEntry(time);
					ceilEntry = map.ceilingEntry(time);
				} else if (o instanceof ITxStream<?>){
					NavigableMap<Long, ?> map = ((ITxStream<?>)o).getEvents();
					floorEntry = map.floorEntry(time);
					ceilEntry = map.ceilingEntry(time);
				} else if(o instanceof ITx){
					NavigableMap<Long, ?> map = ((ITx)o).getStream().getEvents();
					floorEntry = map.floorEntry(time);
					ceilEntry = map.ceilingEntry(time);
				}
				if(floorEntry!=null && time-floorEntry.getKey()>scaling)
					floorEntry=null;
				if(ceilEntry!=null && ceilEntry.getKey()-time>scaling)
					ceilEntry=null;
				if(ceilEntry==null && floorEntry!=null){
					time=floorEntry.getKey();
				}else if(ceilEntry!=null && floorEntry==null){
					time=ceilEntry.getKey();
				}else if(ceilEntry!=null && floorEntry!=null){
					time=time-floorEntry.getKey()<ceilEntry.getKey()-time?floorEntry.getKey(): ceilEntry.getKey();
				}
			}
			return time;
		}
	};

	public TxDisplay(Composite parent) {
		pcs=new PropertyChangeSupport(this);

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

		trackPane = new Composite(topSash, SWT.NONE);
		trackPane.setLayout(new BorderLayout(0, 0));

		waveformList = new WaveformCanvas(trackPane, SWT.NONE);
		waveformList.setLayoutData(BorderLayout.CENTER);

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
		nameList.addMouseListener(nameValueMouseListener);
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
		valueList.addMouseListener(nameValueMouseListener);
		valueListScrolled.setContent(valueList);

		waveformList.setStreams(streams);
		// order is important: it is bottom to top
		waveformList.addPainter(new TrackPainter(waveformList));
		waveformList.addPainter(new RulerPainter(
				waveformList, waveformList.getDisplay().getSystemColor(SWT.COLOR_BLACK), waveformList.getDisplay().getSystemColor(SWT.COLOR_WHITE)));
		CursorPainter cp = new CursorPainter(waveformList, waveformList.getScaleFactor() * 10, cursorPainters.size()-1);
		waveformList.addPainter(cp);
		cursorPainters.add(cp);
		CursorPainter marker = new CursorPainter(waveformList, waveformList.getScaleFactor() * 100, cursorPainters.size()-1);
		waveformList.addPainter(marker);
		cursorPainters.add(marker);
		waveformList.setMaxTime(1);
		waveformList.addMouseListener(waveformMouseListener);

		nameListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				Point v = valueListScrolled.getOrigin();
				valueListScrolled.setOrigin(v.x, y);
				Point t = waveformList.getOrigin();
				waveformList.setOrigin(t.x, -y);
			}
		});
		valueListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				nameListScrolled.setOrigin(nameListScrolled.getOrigin().x, y);
				waveformList.setOrigin(waveformList.getOrigin().x, -y);
			}
		});
		waveformList.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = waveformList.getVerticalBar().getSelection();
				nameListScrolled.setOrigin(nameListScrolled.getOrigin().x, y);
				valueListScrolled.setOrigin(valueListScrolled.getOrigin().x, y);
			}
		});
		topSash.setWeights(new int[] { 30, 70 });
		leftSash.setWeights(new int[] { 75, 25 });

		createStreamDragSource(nameList);
		createStreamDragSource(valueList);
		createStreamDropTarget(nameList);
		createStreamDropTarget(valueList);
		createWaveformDragSource(waveformList);
		createWaveformDropTarget(waveformList);
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
		gd_nameLabel.heightHint = waveformList.getRulerHeight() - 2;
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
		trackVerticalHeight = 0;
		int nameMaxWidth = 0;
		int previousHeight = trackVerticalOffset.size() == 0 ? 0 : trackVerticalOffset.lastKey();
		IWaveformPainter painter = null;
		trackVerticalOffset.clear();
		actualValues.clear();
		waveformList.clearAllWavefromPainter();
		boolean even = true;
		boolean clearSelection = true;
		TextLayout tl = new TextLayout(waveformList.getDisplay());
		tl.setFont(nameFontB);
		for (IWaveform<? extends IWaveformEvent> waveform : streams) {
			int height = waveformList.getTrackHeight();
			clearSelection &= (waveform != currentWaveformSelection);
			if (waveform instanceof ITxStream<?>) {
				ITxStream<? extends ITxEvent> stream = (ITxStream<? extends ITxEvent>) waveform;
				height *= stream.getMaxConcurrency();
				painter = new StreamPainter(waveformList, even, height, (ITxStream<? extends ITxEvent>) waveform);
				actualValues.put(stream, "");
			} else if (waveform instanceof ISignal<?>) {
				painter = new SignalPainter(waveformList, even, height, (ISignal<?>) waveform);
				actualValues.put(waveform, "---");
			}
			waveformList.addWavefromPainter(trackVerticalHeight, painter);
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
		updateValueList();
		waveformList.redraw();
		top.layout(new Control[] { valueList, nameList, waveformList });
		if (trackVerticalOffset.isEmpty() || previousHeight > trackVerticalOffset.lastKey()) {
			Point o = waveformList.getOrigin();
			waveformList.setOrigin(o.x, o.y - (previousHeight - trackVerticalOffset.lastKey()));
		}
		if(clearSelection) setSelection(new StructuredSelection());
		/*        System.out.println("updateTracklist() state:");
        for(Entry<Integer, IWaveform<? extends IWaveformEvent>> entry: trackVerticalOffset.entrySet()){
        	System.out.println("    "+entry.getKey()+": " +entry.getValue().getFullName());
        }
		 */    }

	private int calculateValueWidth() {
		TextLayout tl = new TextLayout(waveformList.getDisplay());
		tl.setFont(nameFontB);
		int valueMaxWidth = 0;
		for (String v : actualValues.values()) {
			tl.setText(v);
			valueMaxWidth = Math.max(valueMaxWidth, tl.getBounds().width);
		}
		return valueMaxWidth + 15;
	}

	private void updateValueList(){
		final Long time = getCursorTime();
		for(Entry<IWaveform<? extends IWaveformEvent>, String> entry:actualValues.entrySet()){
			if(entry.getKey() instanceof ISignal){    
				ISignalChange event = ((ISignal<?>)entry.getKey()).getWaveformEventsBeforeTime(time);
				if(event instanceof ISignalChangeSingle){
					entry.setValue("b'"+((ISignalChangeSingle)event).getValue());
				} else if(event instanceof ISignalChangeMulti){
					entry.setValue("h'"+((ISignalChangeMulti)event).getValue().toHexString());
				}
			} else if(entry.getKey() instanceof ITxStream<?>){
				ITxStream<?> stream = (ITxStream<?>) entry.getKey();
				ITx[] resultsList = new ITx[stream.getMaxConcurrency()];
				Entry<Long, List<ITxEvent>> firstTx=stream.getEvents().floorEntry(time);
				if(firstTx!=null){
					do {
						for(ITxEvent evt:firstTx.getValue()){
						    ITx tx=evt.getTransaction();
							if(evt.getType()==ITxEvent.Type.BEGIN && tx.getBeginTime()<=time && tx.getEndTime()>=time){
								if(resultsList[tx.getConcurrencyIndex()]==null)
								resultsList[tx.getConcurrencyIndex()]= evt.getTransaction();
							}
						}
						firstTx=stream.getEvents().lowerEntry(firstTx.getKey());
					}while(firstTx!=null && !isArrayFull(resultsList));
					String value=null;
					for(ITx o:resultsList){
						if(value==null)
							value=new String();
						else
							value+="|";
						if(o!=null) value+=((ITx)o).getGenerator().getName();
					}
					entry.setValue(value);
				}
            }
		}
		valueList.redraw();
	}

	private boolean isArrayFull(Object[] array){
		for(Object o:array){
			if(o==null) return false;
		}
		return true;
	}
	
	@Override
	public void addSelectionChangedListener(ISelectionChangedListener listener) {
		selectionChangedListeners.add(listener);
	}

	@Override
	public void removeSelectionChangedListener(ISelectionChangedListener listener) {
		selectionChangedListeners.remove(listener);
	}

	public Control getControl() {
		return top;
	}

	public Control getNameControl() {
		return nameList;
	}

	public Control getValueControl() {
		return valueList;
	}

	public Control getWaveformControl() {
		return waveformList;
	}

	@Override
	public ISelection getSelection() {
		if (currentTxSelection != null)
			return new StructuredSelection(currentTxSelection);
		else if (currentWaveformSelection != null)
			return new StructuredSelection(currentWaveformSelection);
		else
			return new StructuredSelection();
	}

	@Override
	public void setSelection(ISelection selection) {
		setSelection(selection, false);
	}
	
	@SuppressWarnings("unchecked")
	public void setSelection(ISelection selection, boolean addIfNeeded) {
		boolean selectionChanged = false;
		if (selection instanceof IStructuredSelection) {
			if(((IStructuredSelection) selection).size()==0){
				selectionChanged = currentTxSelection!=null||currentWaveformSelection!=null;                
				currentTxSelection = null;
				currentWaveformSelection = null;
			} else {
				for(Object sel:((IStructuredSelection) selection).toArray()){
					if (sel instanceof ITx && currentTxSelection != sel){
						ITx txSel = (ITx) sel;
						if (streams.contains(((ITx)sel).getStream())) {
							currentTxSelection = txSel;
							currentWaveformSelection = txSel.getStream();
							selectionChanged = true;
						} else if(addIfNeeded){
							streams.add(txSel.getStream());
							currentTxSelection = txSel;
							currentWaveformSelection = txSel.getStream();
							selectionChanged = true;
						}
					} else if (sel instanceof IWaveform<?> && currentWaveformSelection != sel&& streams.contains(sel)) {
						currentTxSelection = null;
						currentWaveformSelection = (IWaveform<? extends IWaveformEvent>) sel;
						selectionChanged = true;
					}            		
				}
			}
		} else {
			if (currentTxSelection != null || currentWaveformSelection != null)
				selectionChanged = true;
			currentTxSelection = null;
			currentWaveformSelection = null;
		}
		if (selectionChanged) {
			waveformList.setSelected(currentTxSelection, currentWaveformSelection);
			nameList.setData(SELECTION, currentWaveformSelection);
			valueList.redraw();
			nameList.redraw();
			Object[] list = selectionChangedListeners.getListeners();
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
				List<ITxEvent> thisEntryList = stream.getEvents().get(currentTxSelection.getBeginTime());
				boolean meFound=false;
				for (ITxEvent evt : thisEntryList) {
					if (evt.getType() == ITxEvent.Type.BEGIN) {
						if(meFound){
							transaction = evt.getTransaction();
							break;
						}
						meFound|= evt.getTransaction().equals(currentTxSelection);
					}
				}
				if (transaction == null){
					Entry<Long, List<ITxEvent>> entry = stream.getEvents().higherEntry(currentTxSelection.getBeginTime());
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
				List<ITxEvent> thisEntryList = stream.getEvents().get(currentTxSelection.getBeginTime());
				boolean meFound=false;
				for (ITxEvent evt :  Lists.reverse(thisEntryList)) {
					if (evt.getType() == ITxEvent.Type.BEGIN) {
						if(meFound){
							transaction = evt.getTransaction();
							break;
						}
						meFound|= evt.getTransaction().equals(currentTxSelection);
					}
				}
				if (transaction == null){
					Entry<Long, List<ITxEvent>> entry = stream.getEvents().lowerEntry(currentTxSelection.getBeginTime());
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

	public void moveCursor(GotoDirection direction) {
		long time = getCursorTime();
		NavigableMap<Long, ?> map=null;
		if(currentWaveformSelection instanceof ITxStream<?>){
			map=((ITxStream<?>) currentWaveformSelection).getEvents();
		} else if(currentWaveformSelection instanceof ISignal<?>){
			map=((ISignal<?>) currentWaveformSelection).getEvents();
		}
		if(map!=null){
			Entry<Long, ?> entry=direction==GotoDirection.PREV?map.lowerEntry(time):map.higherEntry(time);
			if(entry!=null) {
				time=entry.getKey();
				setCursorTime(time);
				waveformList.reveal(time);
				waveformList.redraw();
			}
		}
	
	}

	public List<IWaveform<? extends IWaveformEvent>> getStreamList() {
		return streams;
	}

	public void moveSelected(int i) {
		if(currentWaveformSelection!=null){
			ITx selectedTx=currentTxSelection;
			IWaveform<? extends IWaveformEvent> selectedWaveform=currentWaveformSelection;
			int idx = streams.indexOf(currentWaveformSelection);
			int newIdx=idx+i;
			if(newIdx>=0 && newIdx<streams.size()){
				Collections.swap(streams,idx,newIdx);
				updateTracklist();
				if(selectedTx!=null){
					setSelection(new StructuredSelection(new Object[]{selectedTx, selectedWaveform}));
				} else
					setSelection(new StructuredSelection(selectedWaveform));
			}
		}	
	}


	protected void paintNames(GC gc, Rectangle rect) {
		if (streams.size() > 0) {
			@SuppressWarnings("unchecked")
			IWaveform<? extends IWaveformEvent> wave = (IWaveform<? extends IWaveformEvent>) nameList.getData(SELECTION);
			Integer firstKey = trackVerticalOffset.floorKey(rect.y);
			if (firstKey == null)
				firstKey = trackVerticalOffset.firstKey();
			Integer lastKey = trackVerticalOffset.floorKey(rect.y + rect.height);
			Rectangle subArea = new Rectangle(rect.x, 0, rect.width, waveformList.getTrackHeight());
			if (lastKey == firstKey) {
				IWaveform<? extends IWaveformEvent> w = trackVerticalOffset.get(firstKey);
				if (w instanceof ITxStream<?>)
					subArea.height *= ((ITxStream<?>) w).getMaxConcurrency();
				drawTextFormat(gc, subArea, firstKey, w.getFullName(), w.equals(wave));
			} else {
				for (Entry<Integer, IWaveform<? extends IWaveformEvent>> entry : trackVerticalOffset.subMap(firstKey, true, lastKey, true)
						.entrySet()) {
					IWaveform<? extends IWaveformEvent> w = entry.getValue();
					subArea.height = waveformList.getTrackHeight();
					if (w instanceof ITxStream<?>)
						subArea.height *= ((ITxStream<?>) w).getMaxConcurrency();
					drawTextFormat(gc, subArea, entry.getKey(), w.getFullName(), w.equals(wave));
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
			Rectangle subArea = new Rectangle(rect.x, 0, rect.width, waveformList.getTrackHeight());
			if (lastKey == firstKey) {
				IWaveform<? extends IWaveformEvent> w = trackVerticalOffset.get(firstKey);
				if (w instanceof ITxStream<?>)
					subArea.height *= ((ITxStream<?>) w).getMaxConcurrency();
				drawValue(gc, subArea, firstKey, actualValues.get(w), w.equals(wave));
			} else {
				for (Entry<Integer, IWaveform<? extends IWaveformEvent>> entry : trackVerticalOffset.subMap(firstKey, true, lastKey, true)
						.entrySet()) {
					IWaveform<? extends IWaveformEvent> w = entry.getValue();
					subArea.height = waveformList.getTrackHeight();
					if (w instanceof ITxStream<?>)
						subArea.height *= ((ITxStream<?>) w).getMaxConcurrency();
					drawValue(gc, subArea, entry.getKey(), actualValues.get(w), w.equals(wave));
				}
			}
		}
	}

	protected void drawValue(GC gc, Rectangle subArea, Integer yOffset, String value, boolean highlite) {
		int beginIndex=0;
		for(int offset=0; offset<subArea.height; offset+=waveformList.getTrackHeight()){
			int endIndex=value.indexOf('|', beginIndex);
			String str = endIndex<0?value.substring(beginIndex):value.substring(beginIndex, endIndex);
			drawTextFormat(gc, new Rectangle(subArea.x, subArea.y, subArea.width, waveformList.getTrackHeight()), yOffset+offset, str, highlite);
			beginIndex=endIndex<0?beginIndex:endIndex+1;
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
		gc.drawText(value, subArea.x + 5, subArea.y + yOffset + (waveformList.getTrackHeight() - size.y) / 2, true);
	}

	public long getMaxTime() {
		return waveformList.getMaxTime();
	}

	public void setMaxTime(long maxTime) {
		this.waveformList.setMaxTime(maxTime);
	}

	public void setZoomLevel(int scale) {
		waveformList.setZoomLevel(scale);
		waveformList.reveal(getCursorTime());
	}

	public int getZoomLevel() {
		return waveformList.getZoomLevel();
	}

	public void setCursorTime(long time){
		final Long oldVal= cursorPainters.get(0).getTime();
		cursorPainters.get(0).setTime(time);
		pcs.firePropertyChange(CURSOR_PROPERTY, oldVal, time);
	}

	public void setMarkerTime(long time, int index){
		if(cursorPainters.size()>index+1){
			final Long oldVal= cursorPainters.get(1+index).getTime();
			cursorPainters.get(1+index).setTime(time);
			pcs.firePropertyChange(MARKER_PROPERTY, oldVal, time);
		}
	}

	public long getCursorTime(){
		return cursorPainters.get(0).getTime();   
	}

	public long getActMarkerTime(){
		return getMarkerTime(selectedMarker);
	}

	public long getMarkerTime(int index){
		return cursorPainters.get(index+1).getTime();   
	}

	private void createStreamDragSource(final Canvas canvas) {
		Transfer[] types = new Transfer[] { LocalSelectionTransfer.getTransfer() };
		DragSource dragSource = new DragSource(canvas, DND.DROP_MOVE);
		dragSource.setTransfer(types);
		dragSource.addDragListener(new DragSourceAdapter() {
			public void dragStart(DragSourceEvent event) {
				if (event.y < trackVerticalHeight) {
					// event.data =
					// trackVerticalOffset.floorEntry(event.y).getValue().getFullName();
					event.doit = true;
					LocalSelectionTransfer.getTransfer().setSelection(new StructuredSelection(currentWaveformSelection));
					// System.out.println("dragStart at location "+new
					// Point(event.x, event.y));
				}
			}

			public void dragSetData(DragSourceEvent event) {
				if (LocalSelectionTransfer.getTransfer().isSupportedType(event.dataType)) {
					event.data =getSelection(); 
				}
			}
		});
	}

	private void createStreamDropTarget(final Canvas canvas) {
		Transfer[] types = new Transfer[] { LocalSelectionTransfer.getTransfer() };
		DropTarget dropTarget = new DropTarget(canvas, DND.DROP_MOVE);
		dropTarget.setTransfer(types);

		dropTarget.addDropListener(new DropTargetAdapter() {
			@SuppressWarnings("unchecked")
			public void drop(DropTargetEvent event) {
				if (LocalSelectionTransfer.getTransfer().isSupportedType(event.currentDataType)){
					ISelection sel = LocalSelectionTransfer.getTransfer().getSelection();
					if(sel!=null && sel instanceof IStructuredSelection){
						Object source = ((IStructuredSelection)sel).getFirstElement();
						DropTarget tgt = (DropTarget) event.widget;
						Point dropPoint = ((Canvas) tgt.getControl()).toControl(event.x, event.y);
						Object target = trackVerticalOffset.floorEntry(dropPoint.y).getValue();
						if(source instanceof IWaveform<?> && target instanceof IWaveform<?>){
							IWaveform<? extends IWaveformEvent> srcWave=(IWaveform<? extends IWaveformEvent>) source;
							int srcIdx=streams.indexOf(srcWave);
							streams.remove(source);
							int tgtIdx=streams.indexOf(target);
							if(srcIdx<=tgtIdx) tgtIdx++;
							if(tgtIdx>=streams.size())
								streams.add(srcWave);
							else
								streams.add(tgtIdx, srcWave);
							currentWaveformSelection=srcWave;
							updateTracklist();
						} else if(source instanceof CursorPainter){
							((CursorPainter)source).setTime(0);
							updateValueList();
						}
					}
				}
			}


			public void dropAccept(DropTargetEvent event) {
				Point offset = canvas.toControl(event.x, event.y); 
				if (event.detail != DND.DROP_MOVE || offset.y > trackVerticalOffset.lastKey() + waveformList.getTrackHeight()) {
					event.detail = DND.DROP_NONE;
				}
			}
		});
	}

	private void createWaveformDragSource(final Canvas canvas) {
		Transfer[] types = new Transfer[] { LocalSelectionTransfer.getTransfer() };
		DragSource dragSource = new DragSource(canvas, DND.DROP_MOVE);
		dragSource.setTransfer(types);
		dragSource.addDragListener(new DragSourceAdapter() {
			public void dragStart(DragSourceEvent event) {
				System.out.println("dragStart");
				event.doit = false;
				List<Object> clicked = waveformList.getClicked(new Point(event.x, event.y));
				for(Object o:clicked){
					if(o instanceof CursorPainter){
						LocalSelectionTransfer.getTransfer().setSelection(new StructuredSelection(o));
						((CursorPainter)o).setDragging(true);
						event.doit = true;
						return;
					}
				}
			}

			public void dragSetData(DragSourceEvent event) {
				if (LocalSelectionTransfer.getTransfer().isSupportedType(event.dataType)) {
					event.data=waveformList.getClicked(new Point(event.x, event.y)); 
				}
			}
		});
	}

	private void createWaveformDropTarget(final Canvas canvas) {
		Transfer[] types = new Transfer[] { LocalSelectionTransfer.getTransfer() };
		DropTarget dropTarget = new DropTarget(canvas, DND.DROP_MOVE);
		dropTarget.setTransfer(types);
		dropTarget.addDropListener(new DropTargetAdapter() {
			public void drop(DropTargetEvent event) {
				if (LocalSelectionTransfer.getTransfer().isSupportedType(event.currentDataType)){
					ISelection sel = LocalSelectionTransfer.getTransfer().getSelection();
					if(sel!=null && sel instanceof IStructuredSelection &&
							((IStructuredSelection)sel).getFirstElement() instanceof CursorPainter){
						CursorPainter painter = (CursorPainter)((IStructuredSelection)sel).getFirstElement();
						painter.setDragging(false);
						updateWaveform(canvas, event, painter);
					}
				}
			}

			public void dropAccept(DropTargetEvent event) {
				Point offset = canvas.toControl(event.x, event.y); 
				if (event.detail != DND.DROP_MOVE || offset.y > trackVerticalOffset.lastKey() + waveformList.getTrackHeight()) {
					event.detail = DND.DROP_NONE;
				}
			}

			public void dragOver(DropTargetEvent event){
				ISelection sel = LocalSelectionTransfer.getTransfer().getSelection();
				if(sel!=null && sel instanceof IStructuredSelection &&
						((IStructuredSelection)sel).getFirstElement() instanceof CursorPainter){
					updateWaveform(canvas, event, (CursorPainter) ((IStructuredSelection)sel).getFirstElement());
				}
			}

			protected void updateWaveform(final Canvas canvas, DropTargetEvent event, CursorPainter painter) {
				Point dropPoint = canvas.toControl(event.x, event.y);
				long time = waveformList.getTimeForOffset(dropPoint.x);
				final Long oldVal= painter.getTime();
				painter.setTime(time);
				if(painter.id<0){
					pcs.firePropertyChange(CURSOR_PROPERTY, oldVal, time);
				}else{
					pcs.firePropertyChange(MARKER_PROPERTY, oldVal, time);
					pcs.firePropertyChange(MARKER_PROPERTY+painter.id, oldVal, time);
				}
				canvas.getDisplay().asyncExec(new Runnable() {
					@Override
					public void run() {
						if(!canvas.isDisposed()){
							canvas.redraw();
							updateValueList();
						}
					}
				});
			}
		});

	}

	public void addPropertyChangeListener(PropertyChangeListener listener) {
		this.pcs.addPropertyChangeListener(listener);
	}

	public void addPropertyChangeListener(String propertyName, PropertyChangeListener listener) {
		this.pcs.addPropertyChangeListener(propertyName, listener);
	}

	public PropertyChangeListener[] getPropertyChangeListeners() {
		return this.pcs.getPropertyChangeListeners();
	}

	public PropertyChangeListener[] getPropertyChangeListeners(String propertyName) {
		return this.pcs.getPropertyChangeListeners(propertyName);
	}

	public void removePropertyChangeListener(PropertyChangeListener listener) {
		this.pcs.removePropertyChangeListener(listener);
	}

	public void removePropertyChangeListener(String propertyName, PropertyChangeListener listener) {
		this.pcs.removePropertyChangeListener(propertyName, listener);
	}

	public boolean hasListeners(String propertyName) {
		return this.pcs.hasListeners(propertyName);
	}

	public String getScaledTime(long time) {
		StringBuilder sb = new StringBuilder();
		Double dTime=new Double(time);
		return sb.append(dTime/waveformList.getScaleFactorPow10()).append(waveformList.getUnitStr()).toString();
	}

	public String[] getZoomLevels(){
		String[] res = new String[WaveformCanvas.unitMultiplier.length*WaveformCanvas.unitString.length];
		int index=0;
		for(String unit:WaveformCanvas.unitString){
			for(int factor:WaveformCanvas.unitMultiplier){
				res[index++]= new Integer(factor).toString()+unit;
			}
		}
		return res;
	}
}
