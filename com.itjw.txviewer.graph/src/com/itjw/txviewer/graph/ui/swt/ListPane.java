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
package com.itjw.txviewer.graph.ui.swt;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.ScrolledComposite;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.ScrollBar;
import com.itjw.txviewer.graph.TxEditorPlugin;
import com.itjw.txviewer.graph.data.ITrStreamFacade;

public abstract class ListPane extends Composite implements PaintListener {

	public static final String SELECTION= "StreamSelected";

	private TxEditorPlugin plugin;
	private TxDisplay display;
	ScrolledComposite scroll;
	Composite frame;
	private MouseAdapter mouseLabelListener;
	private List<PropertyChangeListener> listeners = new ArrayList<PropertyChangeListener>();

	public ListPane(Composite parent, TxDisplay txDisplay) {
		super(parent, SWT.NONE);
		FormLayout fl = new FormLayout();
		fl.marginHeight    = 0;
		fl.marginWidth     = 0;
		this.setLayout(fl);
		plugin=TxEditorPlugin.getDefault();
		display=txDisplay;

		Label header=new Label(this, SWT.NONE);
		header.setText(getHeaderValue());
		header.setAlignment(SWT.CENTER);
		FormData fh = new FormData();
		fh.top=fh.left=new FormAttachment(0);
		fh.right=new FormAttachment(100);
		fh.bottom=new FormAttachment(0, WaveImageCanvas.rulerHeight);
		header.setLayoutData(fh);
		header.setBackground(plugin.getColor(TxEditorPlugin.headerBgColor));
		header.setForeground(plugin.getColor(TxEditorPlugin.headerFgColor));

		scroll=new ScrolledComposite(this, SWT.H_SCROLL | SWT.V_SCROLL);
		FormData fd = new FormData();
		fd.top=new FormAttachment(0);
		fd.bottom=new FormAttachment(100);
		fd.left=new FormAttachment(0);
		fd.right=new FormAttachment(100);
		scroll.setLayoutData(fd);
		scroll.setAlwaysShowScrollBars(true);
		scroll.getHorizontalBar().setVisible(true);
		scroll.getVerticalBar().setVisible(false);
		frame = new Composite(scroll, SWT.NONE);
		
		GridLayout gl = new GridLayout(1, false);
		gl.marginHeight    = 0;
		gl.marginWidth     = 0;
		gl.verticalSpacing = 0;
		frame.setLayout(gl);
		scroll.setContent(frame);
		scroll.setExpandVertical(true);
		scroll.setExpandHorizontal(true);
		scroll.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e) {
				updateSize();
			}
	    });
		scroll.pack();
		mouseLabelListener = new MouseAdapter() {
			@Override
			public void mouseUp(MouseEvent e) {
				fireStreamSelected(e.widget.getData());
			}
		};
	}

	public void scrollToY(int y){
		if (scroll.getContent() == null) return;
		Point location = scroll.getContent().getLocation ();
		ScrollBar vBar = scroll.getVerticalBar ();
		vBar.setSelection(y);
		scroll.getContent().setLocation (location.x, -y);
	}

	public void streamListChanged() {
		for(Control ctrl:frame.getChildren()) ctrl.dispose();
        int trackIdx=0;
		for(ITrStreamFacade str: display.getTxEditor().getStreamList()){
			Label l = new Label(frame, SWT.NONE);
			l.setText(getLabelValue(str));
			l.setData(str);
			formatLabel(l, str, trackIdx++);
			l.addMouseListener(mouseLabelListener);
		}
		frame.pack(true);
		frame.layout(true);
		updateSize();
	}
	
	protected void formatLabel(Label l, ITrStreamFacade str, int trackIdx){
//		GridData gd = new GridData(SWT.FILL, SWT.CENTER, true, false);
		GridData gd = new GridData();
		gd.verticalIndent=trackIdx==0?WaveImageCanvas.rulerHeight:0;
		gd.verticalAlignment = SWT.CENTER;
		gd.horizontalAlignment = SWT.FILL;
		gd.heightHint=str.getHeight();
		gd.grabExcessHorizontalSpace=true;
		l.setLayoutData(gd);
		l.setBackground(trackIdx%2==0?plugin.getColor(TxEditorPlugin.trackBgLightColor):plugin.getColor(TxEditorPlugin.trackBgDarkColor));
		l.setSize(0,str.getHeight());
	}
	
	public void paintControl(PaintEvent e){
		e.gc.setBackground(new Color(null, 255, 255, 255));
		e.gc.fillRectangle(e.x, e.y, e.width, 20);
	}

	private void updateSize() {
		Rectangle r = getClientArea();
		Point p = frame.computeSize(SWT.DEFAULT, SWT.DEFAULT);
		scroll.setMinSize(frame.computeSize(r.width>p.x?r.width:p.x, SWT.DEFAULT));
		scroll.getVerticalBar().setVisible(false);
	}
	
	public void addLabelClickListener(PropertyChangeListener listener){
		listeners.add(listener);
	}

	public void removePropertyChangeListener(PropertyChangeListener listener) {
		listeners.remove(listener);
	}

	protected void fireStreamSelected(Object newValue) {
		PropertyChangeEvent event = new PropertyChangeEvent(this, SELECTION, null, newValue);
		for (int i = 0; i < listeners.size(); i++)
			listeners.get(i).propertyChange(event);
	}

	abstract String getHeaderValue();

	abstract String getLabelValue(ITrStreamFacade str);

}