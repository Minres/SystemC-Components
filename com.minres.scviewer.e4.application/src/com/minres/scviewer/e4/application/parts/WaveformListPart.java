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

import javax.annotation.PostConstruct;
import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.di.Focus;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerFilter;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.ToolBar;
import org.eclipse.swt.widgets.ToolItem;
import org.eclipse.wb.swt.ResourceManager;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.e4.application.provider.TxDbContentProvider;
import com.minres.scviewer.e4.application.provider.TxDbLabelProvider;

public class WaveformListPart implements ISelectionChangedListener {

	@Inject IEventBroker eventBroker;

	@Inject	ESelectionService selectionService;

	private Text nameFilter;
	private TableViewer txTableViewer;
	ToolItem appendItem, insertItem;
	WaveformAttributeFilter attributeFilter;
	
	@PostConstruct
	public void createComposite(Composite parent) {
		parent.setLayout(new GridLayout(1, false));

		nameFilter = new Text(parent, SWT.BORDER);
		nameFilter.setMessage("Enter text to filter waveforms");
		nameFilter.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e) {
				attributeFilter.setSearchText(((Text) e.widget).getText());
				txTableViewer.refresh();
			}
		});
		nameFilter.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		
		attributeFilter = new WaveformAttributeFilter();

		txTableViewer = new TableViewer(parent);
		txTableViewer.setContentProvider(new TxDbContentProvider());
		txTableViewer.setLabelProvider(new TxDbLabelProvider());
		txTableViewer.getTable().setLayoutData(new GridData(GridData.FILL_BOTH));
		txTableViewer.addSelectionChangedListener(this);
		txTableViewer.addFilter(attributeFilter);
		
		ToolBar toolBar = new ToolBar(parent, SWT.FLAT | SWT.RIGHT);
		toolBar.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, false, 1, 1));
		toolBar.setBounds(0, 0, 87, 20);

		insertItem = new ToolItem(toolBar, SWT.NONE);
		insertItem.setImage(ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/bullet_plus.png"));
		insertItem.setText("Insert");
		insertItem.setEnabled(false);
		insertItem.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				eventBroker.post(WaveformViewerPart.ADD_WAVEFORM,
						((IStructuredSelection)txTableViewer.getSelection()).toList());

			}
		});
		appendItem = new ToolItem(toolBar, SWT.NONE);
		appendItem.setImage(ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/bullet_plus.png"));
		appendItem.setText("Append");
		appendItem.setEnabled(false);
		appendItem.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				eventBroker.post(WaveformViewerPart.ADD_WAVEFORM,
						((IStructuredSelection)txTableViewer.getSelection()).toList());

			}
		});
	}

	@Focus
	public void setFocus() {
		txTableViewer.getTable().setFocus();
		setSelection(txTableViewer.getSelection());
	}

	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(WaveformViewerPart.ACTIVE_NODE) Object o) {
		txTableViewer.setInput(o);
	}

	@Override
	public void selectionChanged(SelectionChangedEvent event) {
		setSelection(event.getSelection());
	}

	protected void setSelection(ISelection iSelection) {
		IStructuredSelection selection = (IStructuredSelection)iSelection;
		switch(selection.size()){
		case 0:
			appendItem.setEnabled(false);
			insertItem.setEnabled(false);
			break;
		case 1:
			selectionService.setSelection(selection.getFirstElement());
			appendItem.setEnabled(true);
			break;
		default:
			selectionService.setSelection(selection.toList());
			appendItem.setEnabled(true);
			break;
		}
	}

	@Inject
	public void setSelection(@Named(IServiceConstants.ACTIVE_SELECTION) @Optional Object object){
		if(txTableViewer!=null && !insertItem.isDisposed() && !appendItem.isDisposed())
			if(object instanceof ITx && appendItem.isEnabled()){
				insertItem.setEnabled(true);
			} else if(object instanceof IWaveform<?> && appendItem.isEnabled()){
				insertItem.setEnabled(true);
			} else {
				insertItem.setEnabled(false);
			}
	}
	
	public class WaveformAttributeFilter extends ViewerFilter {

		  private String searchString;

		  public void setSearchText(String s) {
		    this.searchString = ".*" + s + ".*";
		  }

		  @Override
		  public boolean select(Viewer viewer, Object parentElement, Object element) {
		    if (searchString == null || searchString.length() == 0) {
		      return true;
		    }
		    IWaveform<?> p = (IWaveform<?>) element;
		    if (p.getName().matches(searchString)) {
		      return true;
		    }
		    return false;
		  }
		}

}