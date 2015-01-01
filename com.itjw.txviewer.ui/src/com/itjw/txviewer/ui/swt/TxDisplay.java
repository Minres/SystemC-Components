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
package com.itjw.txviewer.ui.swt;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import org.eclipse.core.runtime.ListenerList;
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
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowData;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.ScrollBar;
import org.eclipse.wb.swt.SWTResourceManager;

import swing2swt.layout.BorderLayout;

import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.ITransaction;

public class TxDisplay implements PropertyChangeListener, ISelectionProvider, MouseListener{

    private ListenerList listeners = new ListenerList();
    private ITrStream currentStreamSelection;  
    private ITransaction currentSelection;
	private ScrolledComposite valueListScrolled;
	private ScrolledComposite nameListScrolled;
	private Composite nameList;
	private Composite valueList;
	private ScrolledComposite trackListScrolled;
	private Composite trackList;
	private Composite top;
	private ArrayList<ITrStream> streams=new ArrayList<ITrStream>();
	private Composite trackPane;
	private Ruler ruler;
	private HashMap<ITrStream, Track> trackMap = new HashMap<ITrStream, Track>();

    public TxDisplay(Composite parent) {
		top = new Composite(parent, SWT.NONE);
		top.setLayout(new FillLayout(SWT.HORIZONTAL));
		
		SashForm topSash = new SashForm(top, SWT.SMOOTH);
		topSash.setBackground(topSash.getDisplay().getSystemColor( SWT.COLOR_GRAY));
		
		Composite composite = new Composite(topSash, SWT.NONE);
		composite.setLayout(new FillLayout(SWT.HORIZONTAL));
		
		SashForm leftSash = new SashForm(composite, SWT.SMOOTH);
		leftSash.setBackground(leftSash.getDisplay().getSystemColor( SWT.COLOR_GRAY));
//		leftSash.addControlListener(new ControlAdapter() {
//			public void controlResized(ControlEvent e) {
//				recalculateNameBounds();
//				recalculateValueBounds();
//			}
//		});
		Composite namePane = createTextPane(leftSash, "Name");
		namePane.setBackground(namePane.getDisplay().getSystemColor( SWT.COLOR_WIDGET_BACKGROUND));
		namePane.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e) {
				recalculateNameBounds();
			}
		});
	
		nameListScrolled = new ScrolledComposite(namePane, SWT.H_SCROLL | SWT.V_SCROLL);
		nameListScrolled.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, true, true, 1, 1));
		nameListScrolled.setExpandHorizontal(true);
		nameListScrolled.setExpandVertical(true);
		nameList = new Composite(nameListScrolled, SWT.NONE);
		nameList.setLayout(createScrolledLayoutData(false));

		nameListScrolled.setContent(nameList);

		Composite valuePane = createTextPane(leftSash, "Value");
		valuePane.setBackground(valuePane.getDisplay().getSystemColor( SWT.COLOR_WIDGET_BACKGROUND));
		valuePane.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e) {
				recalculateValueBounds();
			}
		});
		valueListScrolled = new ScrolledComposite(valuePane, SWT.H_SCROLL | SWT.V_SCROLL);
		valueListScrolled.setLayoutData(new GridData(SWT.LEFT, SWT.TOP, true, true, 1, 1));
		valueListScrolled.setExpandHorizontal(true);
		valueListScrolled.setExpandVertical(true);
		valueList = new Composite(valueListScrolled, SWT.NONE);
		valueList.setLayout(createScrolledLayoutData(true));
		valueListScrolled.setContent(valueList);

		
		trackPane = new Composite(topSash, SWT.NONE);
		trackPane.setLayout(new BorderLayout(0, 0));
		ruler = new Ruler(trackPane, SWT.NONE, 0);
		ruler.setLayoutData(BorderLayout.NORTH);
		
		trackListScrolled = new ScrolledComposite(trackPane, SWT.H_SCROLL | SWT.V_SCROLL);
		trackListScrolled.setExpandVertical(true);
		trackListScrolled.setExpandHorizontal(true);
		trackList = new Composite(trackListScrolled, SWT.NONE);
		trackList.setLayout(createScrolledLayoutData(false));
		trackListScrolled.setContent(trackList);
        nameListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				valueListScrolled.setOrigin(valueListScrolled.getOrigin().x, y);
				trackListScrolled.setOrigin(trackListScrolled.getOrigin().x, y);
			}
        });
        valueListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				nameListScrolled.setOrigin(nameListScrolled.getOrigin().x, y);
				trackListScrolled.setOrigin(trackListScrolled.getOrigin().x, y);
			}
        });
		trackListScrolled.getVerticalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int y = ((ScrollBar) e.widget).getSelection();
				nameListScrolled.setOrigin(nameListScrolled.getOrigin().x, y);
				valueListScrolled.setOrigin(valueListScrolled.getOrigin().x, y);
			}
        });
		trackListScrolled.getHorizontalBar().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int x = ((ScrollBar) e.widget).getSelection();
				ruler.setStartPoint(x);
			}
        });

		topSash.setWeights(new int[] {30, 70});
		leftSash.setWeights(new int[] {75, 25});
        
		top.layout(true, true);
		streamListChanged();
	}

	protected RowLayout createScrolledLayoutData(boolean center) {
		RowLayout nameListLayout = new RowLayout(SWT.VERTICAL);
		nameListLayout.spacing = 4;
		nameListLayout.marginTop = 0;
		nameListLayout.marginRight = 0;
		nameListLayout.marginLeft = 0;
		nameListLayout.marginBottom = 0;
		nameListLayout.fill = true;
		nameListLayout.wrap = false;
		nameListLayout.center=center;
		return nameListLayout;
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

	public void streamListChanged() {
		LinkedList<ITrStream>toAdd = new LinkedList<ITrStream>();
		toAdd.addAll(streams);
		for(Control child:trackList.getChildren()){
			Track track = (Track) child;
			Control c = (Control)track.getData("NAMEWIDGET");
			ITrStream stream=(ITrStream) track.getData("STREAM");
			if(!streams.contains(stream)){
				track.setVisible(false);
				c.setVisible(false);
			}else{
				toAdd.remove(stream);
				track.setVisible(true);
				c.setVisible(true);
			}
		}
		for(ITrStream stream: toAdd){
			Track track = new Track(trackList,SWT.NONE);
			track.setTransactions(stream.getTransactions());
			track.setData("STREAM", stream);
			track.addMouseListener(this);
			Point trackSize = track.computeSize(SWT.DEFAULT,SWT.DEFAULT);
			
			Label trackName = new Label(nameList, SWT.NONE);
			trackName.setText(stream.getFullName());
			RowData trackNamelayoutData = new RowData(SWT.DEFAULT, trackSize.y);
			trackName.setLayoutData(trackNamelayoutData);
			track.setData("NAMEWIDGET", trackName);
			
			Label trackValue = new Label(valueList, SWT.NONE);
			trackValue.setText("-");
			RowData trackValuelayoutData = new RowData(SWT.DEFAULT, trackSize.y);
			trackValue.setLayoutData(trackValuelayoutData);
			track.setData("VALUEWIDGET", trackValue);
			trackMap.put(stream, track);
		}
		recalculateNameBounds();
		recalculateValueBounds();		
		Point trackSize=trackList.computeSize(SWT.DEFAULT, SWT.DEFAULT);
		trackListScrolled.setMinSize(trackSize);
		top.layout(true, true);
	}

	protected void recalculateValueBounds() {
		if(streams.size()>0){
			Rectangle bounds = valueListScrolled.getParent().getBounds();
			Point size = valueList.computeSize(SWT.DEFAULT, SWT.DEFAULT);
//			System.out.println("Value calc: "+bounds+" / "+size);
			int corr = (int) (2.35*valueListScrolled.getHorizontalBar().getSize().y);
			valueListScrolled.setMinSize(Math.max(bounds.width,  size.x), Math.max(bounds.height-corr,  size.y));
			valueListScrolled.setAlwaysShowScrollBars(true);
			valueListScrolled.getVerticalBar().setVisible(false);
		}
	}

	protected void recalculateNameBounds() {
		if(streams.size()>0){
			Rectangle bounds = nameListScrolled.getParent().getBounds();
			Point size = nameList.computeSize(SWT.DEFAULT, SWT.DEFAULT);
//			System.out.println("Name calc: "+bounds+" / "+size);
			int corr = (int) (2.35*valueListScrolled.getHorizontalBar().getSize().y);
			nameListScrolled.setMinSize(Math.max(bounds.width,  size.x), Math.max(bounds.height-corr,  size.y));
			nameListScrolled.setAlwaysShowScrollBars(true);
			nameListScrolled.getVerticalBar().setVisible(false);
		}
	}

	@Override
	public void propertyChange(PropertyChangeEvent pce) {
		currentSelection=null;
		ITrStream str = (ITrStream)pce.getNewValue();
		if(str instanceof ITrStream)
			currentStreamSelection=(ITrStream)str;
		if(currentStreamSelection!=null)
			setSelection(getSelection());
		else
			setSelection(StructuredSelection.EMPTY);
	}
	
	@Override
	public void addSelectionChangedListener(ISelectionChangedListener listener) {
		listeners.add(listener);		
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
	public void removeSelectionChangedListener(ISelectionChangedListener listener) {
		listeners.remove(listener);  
	}

	@Override
	public void setSelection(ISelection selection) {
		if(selection instanceof IStructuredSelection){
			Object sel =((IStructuredSelection)selection).getFirstElement();
			if(sel instanceof ITransaction && currentSelection!=sel){
				if(currentSelection!=null){
					ITrStream stream = currentSelection.getGenerator().getStream();
					if(trackMap.containsKey(stream)) trackMap.get(stream).highlightTransaction(null);
				}
				currentSelection=(ITransaction) sel;
				ITrStream stream = currentSelection.getGenerator().getStream();
				if(trackMap.containsKey(stream)){
					Transaction trans = trackMap.get(stream).highlightTransaction(currentSelection);
					trackListScrolled.showControl(trans);
				}
				Object[] list = listeners.getListeners();  
				for (int i = 0; i < list.length; i++) {  
					((ISelectionChangedListener) list[i]).selectionChanged(new SelectionChangedEvent(this, selection));  
				}
			}
		}
	}

	@Override
	public void mouseDoubleClick(MouseEvent e) {
	}

	@Override
	public void mouseDown(MouseEvent e) {
		if(e.data!=null){
			StructuredSelection sel = new StructuredSelection(((Transaction)e.data).getData());
			setSelection(sel);
		}else{
			StructuredSelection sel = new StructuredSelection(new Object[]{ ((Track)e.widget).getData()});
			setSelection(sel);
		}
	}

	@Override
	public void mouseUp(MouseEvent e) {		
	}
	
	public boolean addStream(ITrStream paramE){
		boolean res = streams.add(paramE);
		streamListChanged();
		return res;
	}

	public boolean addAllStreams(ITrStream[] streams) {
		boolean res =  this.streams.addAll(Arrays.asList(streams));
		streamListChanged();
		return res;
	}

	public boolean addAllStreams(Collection<? extends ITrStream> paramCollection){
		boolean res =  streams.addAll(paramCollection);
		streamListChanged();
		return res;
	}

	public boolean removeStream(ITrStream paramObject){
		boolean res =  streams.remove(paramObject);
		streamListChanged();
		return res;
	}

	public boolean removeAllStreams(Collection<?> paramCollection){
		boolean res =  streams.removeAll(paramCollection);
		streamListChanged();
		return res;
	}

	public List<ITrStream> getStreamList(){
		return Collections.unmodifiableList(streams);
	}

	public boolean removeAllStreams(ITrStream[] streams) {
		boolean res =  this.streams.removeAll(Arrays.asList(streams));
		streamListChanged();
		return res;
	}

}
