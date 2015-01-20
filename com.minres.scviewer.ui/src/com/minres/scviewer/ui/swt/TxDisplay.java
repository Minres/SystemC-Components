/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.ui.swt;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.NavigableMap;
import java.util.Map.Entry;
import java.util.TreeMap;

import org.eclipse.core.runtime.ListenerList;
import org.eclipse.jface.resource.FontDescriptor;
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
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.ControlListener;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.graphics.TextLayout;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.wb.swt.SWTResourceManager;

import swing2swt.layout.BorderLayout;

import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.ui.handler.GotoDirection;

public class TxDisplay implements PropertyChangeListener, ISelectionProvider, MouseListener{
	private ListenerList listeners = new ListenerList();
    private IWaveform currentStreamSelection;  
    private ITx currentSelection;
    private IWaveform currentWaveformSelection;

    private ScrolledComposite nameListScrolled;
	private ScrolledComposite valueListScrolled;

	private Canvas nameList;
	private Canvas valueList;
	private WaveformCanvas trackList;

	private Composite top;
	ObservableList<IWaveform> streams;
	private Composite trackPane;
	private Ruler ruler;
	TreeMap<Integer, IWaveform> trackVerticalOffset;
	
//	private long maxTime=0; 
	private Font nameFont;
	private Font valueFont;
	
    public TxDisplay(Composite parent) {
    	trackVerticalOffset = new TreeMap<Integer, IWaveform>();
    	Display d =parent.getDisplay();
    	parent.addDisposeListener(new DisposeListener() {
			@Override
			public void widgetDisposed(DisposeEvent e) {
				dispose();
			}
		});
    	FontDescriptor fontDescriptor = FontDescriptor.createFrom(d.getSystemFont()).setStyle(SWT.BOLD);
    	nameFont = fontDescriptor.createFont(d);
    	valueFont = fontDescriptor.createFont(d);
    	
    	streams=new ObservableList<IWaveform>();
    	streams.addPropertyChangeListener(this);
    	
		top = new Composite(parent, SWT.NONE);
		top.setLayout(new FillLayout(SWT.HORIZONTAL));
		
		SashForm topSash = new SashForm(top, SWT.SMOOTH);
		topSash.setBackground(topSash.getDisplay().getSystemColor( SWT.COLOR_GRAY));
		
		Composite composite = new Composite(topSash, SWT.NONE);
		composite.setLayout(new FillLayout(SWT.HORIZONTAL));
		
		SashForm leftSash = new SashForm(composite, SWT.SMOOTH);
		leftSash.setBackground(leftSash.getDisplay().getSystemColor( SWT.COLOR_GRAY));

		Composite namePane = createTextPane(leftSash, "Name");
		namePane.setBackground(namePane.getDisplay().getSystemColor( SWT.COLOR_WIDGET_BACKGROUND));
	
		nameListScrolled = new ScrolledComposite(namePane, SWT.H_SCROLL | SWT.V_SCROLL);
		nameListScrolled.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));
		nameListScrolled.setAlwaysShowScrollBars(true);
		nameListScrolled.addControlListener(new ControlAdapter(){
			@Override
			public void controlResized(ControlEvent e) {
				nameListScrolled.getVerticalBar().setVisible(false);
				
			}
		});
		nameList = new Canvas(nameListScrolled, SWT.NONE){
			@Override
			public Point computeSize(int wHint, int hHint, boolean changed) {
				Rectangle bounds= super.getClientArea();
				return new Point(bounds.width, bounds.height);
			}
		};
		nameList.addListener(SWT.Paint, new Listener() {
			@Override
			public void handleEvent(Event event) {
				GC gc = event.gc;
	            Rectangle rect = ((Canvas)event.widget).getClientArea();
	            paintNames(gc, rect);				
			}
		});
		nameList.addMouseListener(this);
		nameListScrolled.setContent(nameList);

		Composite valuePane = createTextPane(leftSash, "Value");
		valuePane.setBackground(valuePane.getDisplay().getSystemColor( SWT.COLOR_WIDGET_BACKGROUND));
		valueListScrolled = new ScrolledComposite(valuePane, SWT.H_SCROLL | SWT.V_SCROLL);
		valueListScrolled.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));
		valueListScrolled.setAlwaysShowScrollBars(true);
		valueListScrolled.addControlListener(new ControlAdapter(){
			@Override
			public void controlResized(ControlEvent e) {
				valueListScrolled.getVerticalBar().setVisible(false);
				
			}
		});
		valueList = new Canvas(valueListScrolled, SWT.NONE){
			@Override
			public Point computeSize(int wHint, int hHint, boolean changed) {
				Rectangle bounds= super.getClientArea();
				return new Point(bounds.width, bounds.height);
			}
		};
		valueList.addListener(SWT.Paint, new Listener() {
			@Override
			public void handleEvent(Event event) {
				GC gc = event.gc;
	            Rectangle rect = ((Canvas)event.widget).getClientArea();
	            paintValues(gc, rect);				
			}
		});
		valueList.addMouseListener(this);
		valueListScrolled.setContent(valueList);
		
		trackPane = new Composite(topSash, SWT.NONE);
		trackPane.setLayout(new BorderLayout(0, 0));
		ruler = new Ruler(trackPane, SWT.NONE, 0);
		ruler.setLayoutData(BorderLayout.NORTH);

		trackList = new WaveformCanvas(trackPane, SWT.NONE);
		trackList.setLayoutData(BorderLayout.CENTER);
		trackList.streams=streams;
		trackList.addTrackPainter(new TrackPainter(trackList));
		trackList.setMaxTime(1);
		trackList.addMouseListener(this);
		
		nameListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
 			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				valueListScrolled.setOrigin(valueListScrolled.getOrigin().x, y);
				trackList.scrollToY(y);
			}
        });
        valueListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				nameListScrolled.setOrigin(nameListScrolled.getOrigin().x, y);
				trackList.scrollToY(y);
			}
        });
        trackList.getHorizontalBar().addSelectionListener(new SelectionAdapter() {
        	public void widgetSelected(SelectionEvent e) {
        		ruler.setStartPoint(trackList.getHorizontalBar().getSelection());
        	}
		});
        trackList.getVerticalBar().addSelectionListener(new SelectionAdapter() {
        	public void widgetSelected(SelectionEvent e) {
				int y = trackList.getVerticalBar().getSelection();
				nameListScrolled.setOrigin(nameListScrolled.getOrigin().x, y);
				valueListScrolled.setOrigin(valueListScrolled.getOrigin().x, y);
        	}
		});
		topSash.setWeights(new int[] {30, 70});
		leftSash.setWeights(new int[] {75, 25});
	}


	protected void dispose() {
		nameFont.dispose();
		valueFont.dispose();
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
		gd_nameLabel.heightHint = Ruler.height-2;
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
		if("size".equals(pce.getPropertyName())) {
			updateTracklist();
		}
	}

	protected void updateTracklist() {
		int yoffs=0;
		int nameMaxWidth=0;
		int valueMaxWidth=0;
		
		IWaveformPainter painter=null;
		trackVerticalOffset.clear();
		trackList.clearAllWavefromPainter();
		boolean even=true;
		TextLayout tl = new TextLayout(trackList.getDisplay());
		for(IWaveform waveform:streams){
			int height=trackList.getTrackHeight();
			if(waveform instanceof ITxStream){
				height*=((ITxStream)waveform).getMaxConcurrency();
				painter= new StreamPainter(trackList, even, height, (ITxStream) waveform);
			} else if(waveform instanceof ISignal<?>){
				painter= new SignalPainter(trackList, even, height, (ISignal<?>) waveform);
			}
			trackList.addWavefromPainter(yoffs, painter);
			trackVerticalOffset.put(yoffs, waveform);
			tl.setText(waveform.getFullName());
			nameMaxWidth=Math.max(nameMaxWidth, tl.getBounds().width);
			valueMaxWidth=nameMaxWidth;
			yoffs+=height;
			even=!even;
		}
		valueList.setSize(nameMaxWidth, yoffs);
		nameList.setSize(valueMaxWidth, yoffs);
		valueList.redraw();
		nameList.redraw();
		trackList.redraw();
		top.layout(new Control[]{valueList, nameList, trackList});
	}
	
	@Override
	public void addSelectionChangedListener(ISelectionChangedListener listener) {
		listeners.add(listener);		
	}

	@Override
	public void removeSelectionChangedListener(ISelectionChangedListener listener) {
		listeners.remove(listener);  
	}

	@Override
	public ISelection getSelection() {
		if(currentSelection!=null)
			return new StructuredSelection(currentSelection);
		else if(currentStreamSelection!=null)
			return new StructuredSelection(currentStreamSelection);
		else
			return null;
	}

	@Override
	public void setSelection(ISelection selection) {
		boolean selectionChanged=false;
		if(selection instanceof IStructuredSelection){
			Object sel =((IStructuredSelection)selection).getFirstElement();
			if(sel instanceof ITx && currentSelection!=sel){
				currentSelection=(ITx) sel;
				currentWaveformSelection = currentSelection.getStream();
				selectionChanged=true;
			} else if(sel instanceof IWaveform && currentStreamSelection!=sel){
				currentSelection=null;
				currentWaveformSelection = (IWaveform) sel;
				selectionChanged=true;
			}
		} else {
			if(currentSelection!=null || currentWaveformSelection!=null) selectionChanged=true;
			currentSelection=null;
			currentWaveformSelection = null;			
		}
		if(selectionChanged){
			Object[] list = listeners.getListeners();  
			for (int i = 0; i < list.length; i++) {  
				((ISelectionChangedListener) list[i]).selectionChanged(new SelectionChangedEvent(this, selection));  
			}
		}
	}

	public void moveSelection(GotoDirection direction) {
		if(currentStreamSelection instanceof ITxStream<?>){
			ITxStream<ITxEvent> stream = (ITxStream<ITxEvent>) currentStreamSelection;
			ITx transaction=null;
			if(direction==GotoDirection.NEXT){
				Entry<Long, Collection<ITxEvent>> entry = stream.getEvents().higherEntry(currentSelection.getBeginTime());
				if(entry!=null)
					transaction = entry.getValue().iterator().next().getTransaction();
			}else if(direction==GotoDirection.PREV){
				Entry<Long, Collection<ITxEvent>> entry = stream.getEvents().lowerEntry(currentSelection.getBeginTime());
				if(entry!=null)
					transaction = entry.getValue().iterator().next().getTransaction();
			}
			if(transaction!=null)
				setSelection(new StructuredSelection(transaction));				
		}
	}

	@Override
	public void mouseDoubleClick(MouseEvent e) {
	}

	@Override
	public void mouseDown(MouseEvent e) {
		if(e.widget==trackList){
			
		}else if(e.widget==valueList){
			
		}else if(e.widget==nameList){
			
		}
	}

	@Override
	public void mouseUp(MouseEvent e) {		
	}
	
	public List<IWaveform> getStreamList(){
		return streams;
	}

	protected void paintNames(GC gc, Rectangle rect) {
		if(streams.size()>0){
			Integer firstKey=trackVerticalOffset.floorKey(rect.y);
			if(firstKey==null) firstKey=trackVerticalOffset.firstKey();
			Integer lastKey = trackVerticalOffset.floorKey(rect.y+rect.height);
			Rectangle subArea = new Rectangle(rect.x, 0, rect.width, 0);
			if(lastKey==firstKey){
				drawTextFormat(gc, subArea, firstKey, trackVerticalOffset.get(firstKey).getFullName());
			}else{
				for(Entry<Integer, IWaveform> entry : trackVerticalOffset.subMap(firstKey, true, lastKey, true).entrySet()){
					drawTextFormat(gc, subArea, entry.getKey(), entry.getValue().getFullName());
				}
			}
		}		
	}

	protected void paintValues(GC gc, Rectangle rect) {
		if(streams.size()>0){
			Integer firstKey=trackVerticalOffset.floorKey(rect.y);
			if(firstKey==null) firstKey=trackVerticalOffset.firstKey();
			Integer lastKey = trackVerticalOffset.floorKey(rect.y+rect.height);
			Rectangle subArea = new Rectangle(rect.x, 0, rect.width, 0);
			if(lastKey==firstKey){
				drawTextFormat(gc, subArea, firstKey, trackVerticalOffset.get(firstKey).getFullName());
			}else{
				for(Entry<Integer, IWaveform> entry : trackVerticalOffset.subMap(firstKey, true, lastKey, true).entrySet()){
					drawTextFormat(gc, subArea, entry.getKey(), "---");
				}
			}
		}		
	}

	protected void drawTextFormat(GC gc, Rectangle subArea, int yOffset, String p) {
		Point size = gc.textExtent(p);
		gc.drawText(p, subArea.x, subArea.y + yOffset+(trackList.getTrackHeight()-size.y)/2, true);
	}

	public long getMaxTime() {
		return trackList.getMaxTime();
	}

	public void setMaxTime(long maxTime) {
		this.trackList.setMaxTime(maxTime);
	}

}
