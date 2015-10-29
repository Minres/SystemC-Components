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
package com.minres.scviewer.e4.application.parts;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Arrays;
import java.util.List;

import javax.annotation.PostConstruct;
import javax.inject.Inject;

import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.di.Focus;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;

import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.e4.application.provider.TxDbContentProvider;
import com.minres.scviewer.e4.application.provider.TxDbLabelProvider;

public class DesignBrowser implements ISelectionChangedListener {

	@Inject IEventBroker eventBroker;
	
	@Inject	ESelectionService selectionService;

	private TreeViewer treeViewer;


	private PropertyChangeListener l = new PropertyChangeListener() {
		@Override
		public void propertyChange(PropertyChangeEvent evt) {
			if("CHILDS".equals(evt.getPropertyName())){
				treeViewer.getTree().getDisplay().asyncExec(new Runnable() {					
					@Override
					public void run() {
						treeViewer.refresh();
					}
				});
			}
		}
	};

	
	@PostConstruct
	public void createComposite(Composite parent) {
		parent.setLayout(new GridLayout(1, false));
		treeViewer = new TreeViewer(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL);
		treeViewer.addSelectionChangedListener(this);
		treeViewer.getTree().setLayoutData(new GridData(GridData.FILL_BOTH));
		treeViewer.setContentProvider(new TxDbContentProvider());
		treeViewer.setLabelProvider(new TxDbLabelProvider());
		treeViewer.setUseHashlookup(true);
		treeViewer.setAutoExpandLevel(2);
	}

	@Focus
	public void setFocus() {
		treeViewer.getTree().setFocus();
		selectionService.setSelection(treeViewer.getSelection());
	}

	@Override
	public void selectionChanged(SelectionChangedEvent event) {
		selectionService.setSelection(event.getSelection());
	}

	@SuppressWarnings("unchecked")
	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(WaveformViewerPart.ACTIVE_WAVEFORMVIEW) WaveformViewerPart waveformViewerPart) {
		IWaveformDb database = waveformViewerPart.getDatabase();
		Object input = treeViewer.getInput();
		if(input!=null && input instanceof List<?>)
			((List<IWaveformDb>)input).get(0).removePropertyChangeListener(l);
		treeViewer.setInput(database.isLoaded()?Arrays.asList(new IWaveformDb[]{database}):null);
		// Set up the tree viewer
		database.addPropertyChangeListener(l);
	} 
/*
 * 	TODO: needs top be implemented
	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(WaveformViewerPart.ACTIVE_NODE_PATH) String path) {
		
	}
*/
};