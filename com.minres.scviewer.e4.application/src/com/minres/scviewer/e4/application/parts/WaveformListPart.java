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

import java.lang.annotation.Annotation;

import javax.annotation.PostConstruct;
import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.e4.core.contexts.ContextInjectionFactory;
import org.eclipse.e4.core.contexts.IEclipseContext;
import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.di.Focus;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.services.EMenuService;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;
import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredSelection;
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

import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.e4.application.handlers.AddWaveformHandler;
import com.minres.scviewer.e4.application.provider.TxDbContentProvider;
import com.minres.scviewer.e4.application.provider.TxDbLabelProvider;

public class WaveformListPart implements ISelectionChangedListener {

	private static final String POPUP_ID="com.minres.scviewer.e4.application.parts.WaveformList.popupmenu";

	@Inject IEventBroker eventBroker;

	@Inject	ESelectionService selectionService;

	@Inject EMenuService menuService;

	@Inject IEclipseContext eclipseCtx;


	private Text nameFilter;

	private TableViewer txTableViewer;

	ToolItem appendItem, insertItem, insertAllItem, appendAllItem;

	WaveformAttributeFilter attributeFilter;

	int thisSelectionCount=0, otherSelectionCount=0;

	private WaveformViewerPart waveformViewerPart;

	@PostConstruct
	public void createComposite(Composite parent) {
		parent.setLayout(new GridLayout(1, false));

		nameFilter = new Text(parent, SWT.BORDER);
		nameFilter.setMessage("Enter text to filter waveforms");
		nameFilter.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e) {
				attributeFilter.setSearchText(((Text) e.widget).getText());
				updateButtons();
				txTableViewer.refresh();
			}
		});
		nameFilter.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

		attributeFilter = new WaveformAttributeFilter();

		txTableViewer = new TableViewer(parent);
		txTableViewer.setContentProvider(new TxDbContentProvider(true));
		txTableViewer.setLabelProvider(new TxDbLabelProvider());
		txTableViewer.getTable().setLayoutData(new GridData(GridData.FILL_BOTH));
		txTableViewer.addSelectionChangedListener(this);
		txTableViewer.addFilter(attributeFilter);
		txTableViewer.addDoubleClickListener(new IDoubleClickListener() {
			@Override
			public void doubleClick(DoubleClickEvent event) {
				AddWaveformHandler myHandler = new AddWaveformHandler();
				Object result = runCommand(myHandler, CanExecute.class, "after", false);
				if(result!=null && (Boolean)result)
					ContextInjectionFactory.invoke(myHandler, Execute.class, eclipseCtx);
			}
		});
		menuService.registerContextMenu(txTableViewer.getControl(), POPUP_ID);

		ToolBar toolBar = new ToolBar(parent, SWT.FLAT | SWT.RIGHT);
		toolBar.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, false, 1, 1));
		toolBar.setBounds(0, 0, 87, 20);

		appendItem = new ToolItem(toolBar, SWT.NONE);
		appendItem.setToolTipText("Append after");
		appendItem.setImage(ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/append_wave.png"));
		appendItem.setEnabled(false);
		appendItem.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				/*
				eventBroker.post(WaveformViewerPart.ADD_WAVEFORM,
						((IStructuredSelection)txTableViewer.getSelection()).toList());
				ECommandService commandService = eclipseCtx.get(ECommandService.class);
				EHandlerService handlerService = eclipseCtx.get(EHandlerService.class); 
				HashMap<String,Object> param=new HashMap<>();
				param.clear();
				//param.put("where", "after");
				ParameterizedCommand myCommand = commandService.createCommand(COMMAND_ID, param);
				if(myCommand!=null)handlerService.executeHandler(myCommand);
				 */
				AddWaveformHandler myHandler = new AddWaveformHandler();
				Object result = runCommand(myHandler, CanExecute.class, "after", false);
				if(result!=null && (Boolean)result)
					ContextInjectionFactory.invoke(myHandler, Execute.class, eclipseCtx);
			}

		});

		insertItem = new ToolItem(toolBar, SWT.NONE);
		insertItem.setToolTipText("Insert before");
		insertItem.setImage(ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/insert_wave.png"));
		insertItem.setEnabled(false);
		insertItem.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				AddWaveformHandler myHandler = new AddWaveformHandler();
				Object result = runCommand(myHandler, CanExecute.class, "before", false);
				if(result!=null && (Boolean)result)
					ContextInjectionFactory.invoke(myHandler, Execute.class, eclipseCtx);
			}
		});
		new ToolItem(toolBar, SWT.SEPARATOR);

		appendAllItem = new ToolItem(toolBar, SWT.NONE);
		appendAllItem.setToolTipText("Append all after");
		appendAllItem.setImage(ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/append_all_waves.png"));
		appendAllItem.setEnabled(false);

		new ToolItem(toolBar, SWT.SEPARATOR);
		appendAllItem.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				Object[] all = getFilteredChildren(txTableViewer);
				if(all.length>0){
					Object oldSel=selectionService.getSelection();
					selectionService.setSelection(new StructuredSelection(all));
					AddWaveformHandler myHandler = new AddWaveformHandler();
					Object result = runCommand(myHandler, CanExecute.class, "after", false);
					if(result!=null && (Boolean)result)
						ContextInjectionFactory.invoke(myHandler, Execute.class, eclipseCtx);
					selectionService.setSelection(oldSel);
				}
			}
		});
		insertAllItem = new ToolItem(toolBar, SWT.NONE);
		insertAllItem.setToolTipText("Insert all before");
		insertAllItem.setImage(ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/insert_all_waves.png"));
		insertAllItem.setEnabled(false);
		insertAllItem.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				Object[] all = getFilteredChildren(txTableViewer);
				if(all.length>0){
					Object oldSel=selectionService.getSelection();
					selectionService.setSelection(new StructuredSelection(all));
					AddWaveformHandler myHandler = new AddWaveformHandler();
					Object result = runCommand(myHandler, CanExecute.class, "before", false);
					if(result!=null && (Boolean)result)
						ContextInjectionFactory.invoke(myHandler, Execute.class, eclipseCtx);
					selectionService.setSelection(oldSel);
				}
			}
		});
	}

	@Focus
	public void setFocus() {
		txTableViewer.getTable().setFocus();
		setSelection(txTableViewer.getSelection());
	}

	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(WaveformViewerPart.ACTIVE_WAVEFORMVIEW) WaveformViewerPart part) {
		this.waveformViewerPart=part;
		updateButtons();
	}

	@Override
	public void selectionChanged(SelectionChangedEvent event) {
		setSelection(event.getSelection());
	}

	protected void setSelection(ISelection iSelection) {
		IStructuredSelection selection = (IStructuredSelection)iSelection;
		if(selection.size()==0){
			appendItem.setEnabled(false);
		}
		selectionService.setSelection(selection);
		thisSelectionCount=selection.toList().size();
		updateButtons();
	}

	@Inject
	public void setSelection(@Named(IServiceConstants.ACTIVE_SELECTION) @Optional IStructuredSelection selection, EPartService partService){
		MPart part = partService.getActivePart();
		if(part!=null && part.getObject() != this && selection!=null){
			if( selection instanceof IStructuredSelection) {
				Object object= ((IStructuredSelection)selection).getFirstElement();			
				if(object instanceof IHierNode&& ((IHierNode)object).getChildNodes().size()!=0)
					txTableViewer.setInput(object);
				otherSelectionCount = (object instanceof IWaveform<?> || object instanceof ITx)?1:0;
			}
		}
		updateButtons();
	}

	private void updateButtons() {
		if(txTableViewer!=null && !insertItem.isDisposed() && !appendItem.isDisposed() && 
				!appendAllItem.isDisposed() && !insertAllItem.isDisposed()){
			AddWaveformHandler myHandler = new AddWaveformHandler();
			Object result = runCommand(myHandler, CanExecute.class, "after", false);
			appendItem.setEnabled(result instanceof Boolean && (Boolean)result);
			result = runCommand(myHandler, CanExecute.class, "after", true);
			appendAllItem.setEnabled(result instanceof Boolean && (Boolean)result);
			result = runCommand(myHandler, CanExecute.class, "before", false);
			insertItem.setEnabled(result instanceof Boolean && (Boolean)result);
			result = runCommand(myHandler, CanExecute.class, "before", true);
			insertAllItem.setEnabled(result instanceof Boolean && (Boolean)result);
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

	public Object[] getFilteredChildren(){
		return getFilteredChildren(txTableViewer);
	}
	
	protected Object[] getFilteredChildren(TableViewer viewer){
		Object parent = viewer.getInput();
		if(parent==null) return new Object[0];
		Object[] result = null;
		if (parent != null) {
			IStructuredContentProvider cp = (IStructuredContentProvider) viewer.getContentProvider();
			if (cp != null) {
				result = cp.getElements(parent);
				if(result==null) return new Object[0];
				for (int i = 0, n = result.length; i < n; ++i) {
					if(result[i]==null) return new Object[0];
				}
			}
		}
		ViewerFilter[] filters = viewer.getFilters();
		if (filters != null) {
			for (ViewerFilter f:filters) {
				Object[] filteredResult = f.filter(viewer, parent, result);
				result = filteredResult;
			}
		}
		return result;
	}

	public WaveformViewerPart getActiveWaveformViewerPart() {
		return waveformViewerPart;
	}

	protected Object runCommand(AddWaveformHandler myHandler, Class<? extends Annotation> annotation, String where, Boolean all) {
		ContextInjectionFactory.inject(myHandler, eclipseCtx);
		eclipseCtx.set(AddWaveformHandler.PARAM_WHERE_ID, where);
		eclipseCtx.set(AddWaveformHandler.PARAM_ALL_ID, all.toString());
		Object result = ContextInjectionFactory.invoke(myHandler, annotation, eclipseCtx);
		return result;
	}

}