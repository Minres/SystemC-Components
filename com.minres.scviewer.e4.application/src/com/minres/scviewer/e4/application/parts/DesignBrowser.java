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
import java.lang.annotation.Annotation;
import java.util.Arrays;
import java.util.List;

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
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerFilter;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.ToolBar;
import org.eclipse.swt.widgets.ToolItem;
import org.eclipse.wb.swt.ResourceManager;
import org.eclipse.wb.swt.SWTResourceManager;

import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.e4.application.handlers.AddWaveformHandler;
import com.minres.scviewer.e4.application.provider.TxDbContentProvider;
import com.minres.scviewer.e4.application.provider.TxDbLabelProvider;

/**
 * The Class DesignBrowser. It contains the design tree, a list of Streams & signals and a few buttons to 
 * add them them to the waveform view 
 */
public class DesignBrowser {

	/** The Constant POPUP_ID. */
	private static final String POPUP_ID="com.minres.scviewer.e4.application.parts.DesignBrowser.popupmenu";

	/** The event broker. */
	@Inject IEventBroker eventBroker;
	
	/** The selection service. */
	@Inject	ESelectionService selectionService;

	/** The menu service. */
	@Inject EMenuService menuService;

	/** The eclipse ctx. */
	@Inject IEclipseContext eclipseCtx;
	
	/** The sash form. */
	private SashForm sashForm;
	
	/** The top. */
	Composite top;

	/** The bottom. */
	private Composite bottom;
	
	/** The tree viewer. */
	private TreeViewer treeViewer;

	/** The name filter. */
	private Text nameFilter;

	/** The tx table viewer. */
	private TableViewer txTableViewer;

	/** The append all item. */
	ToolItem appendItem, insertItem, insertAllItem, appendAllItem;

	/** The attribute filter. */
	WaveformAttributeFilter attributeFilter;

	/** The other selection count. */
	int thisSelectionCount=0, otherSelectionCount=0;

	/** The tree viewer pcl. */
	private PropertyChangeListener treeViewerPCL = new PropertyChangeListener() {
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

	/** The waveform viewer part. */
	private WaveformViewer waveformViewerPart;

	/** The sash paint listener. */
	protected PaintListener sashPaintListener=new PaintListener() {					
		@Override
		public void paintControl(PaintEvent e) {
			int size=Math.min(e.width, e.height)-1;
			e.gc.setForeground(SWTResourceManager.getColor(SWT.COLOR_DARK_GRAY));
			e.gc.setFillRule(SWT.FILL_EVEN_ODD);
			if(e.width>e.height)
				e.gc.drawArc(e.x+(e.width-size)/2, e.y, size, size, 0, 360);
			else
				e.gc.drawArc(e.x, e.y+(e.height-size)/2, size, size, 0, 360);
		}
	};

	
	/**
	 * Creates the composite.
	 *
	 * @param parent the parent
	 */
	@PostConstruct
	public void createComposite(Composite parent) {
		sashForm = new SashForm(parent, SWT.BORDER | SWT.SMOOTH | SWT.VERTICAL);

		top = new Composite(sashForm, SWT.NONE);
		createTreeViewerComposite(top);
		bottom = new Composite(sashForm, SWT.NONE);
		createTableComposite(bottom);
		
		sashForm.setWeights(new int[] {100, 100});
		sashForm.SASH_WIDTH=5;
		top.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e) {
				sashForm.getChildren()[2].addPaintListener(sashPaintListener);
				top.removeControlListener(this);
			}
		});
	}
	
	/**
	 * Creates the tree viewer composite.
	 *
	 * @param parent the parent
	 */
	public void createTreeViewerComposite(Composite parent) {
		parent.setLayout(new GridLayout(1, false));
		treeViewer = new TreeViewer(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL);
		treeViewer.getTree().setLayoutData(new GridData(GridData.FILL_BOTH));
		treeViewer.setContentProvider(new TxDbContentProvider());
		treeViewer.setLabelProvider(new TxDbLabelProvider());
		treeViewer.setUseHashlookup(true);
		treeViewer.setAutoExpandLevel(2);
		treeViewer.addSelectionChangedListener(new ISelectionChangedListener() {
			
			@Override
			public void selectionChanged(SelectionChangedEvent event) {
				ISelection selection=event.getSelection();
				if( selection instanceof IStructuredSelection) {
					Object object= ((IStructuredSelection)selection).getFirstElement();			
					if(object instanceof IHierNode&& ((IHierNode)object).getChildNodes().size()!=0){
						txTableViewer.setInput(object);
						updateButtons();
					}
				}
			}
		});
	}

	/**
	 * Creates the table composite.
	 *
	 * @param parent the parent
	 */
	public void createTableComposite(Composite parent) {
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
		txTableViewer.addSelectionChangedListener(new ISelectionChangedListener() {
			
			@Override
			public void selectionChanged(SelectionChangedEvent event) {
					selectionService.setSelection(event.getSelection());
					updateButtons();
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

	/**
	 * Sets the focus.
	 */
	@Focus
	public void setFocus() {
		txTableViewer.getTable().setFocus();
		IStructuredSelection selection = (IStructuredSelection)txTableViewer.getSelection();
		if(selection.size()==0){
			appendItem.setEnabled(false);
		}
		selectionService.setSelection(selection);
		thisSelectionCount=selection.toList().size();
		updateButtons();
	}

	/**
	 * Gets the status event.
	 *
	 * @param waveformViewerPart the waveform viewer part
	 * @return the status event
	 */
	@SuppressWarnings("unchecked")
	@Inject @Optional
	public void  getActiveWaveformViewerEvent(@UIEventTopic(WaveformViewer.ACTIVE_WAVEFORMVIEW) WaveformViewer waveformViewerPart) {
		if(this.waveformViewerPart!=null)
			this.waveformViewerPart.storeDesignBrowerState(new DBState());
		this.waveformViewerPart=waveformViewerPart;
		IWaveformDb database = waveformViewerPart.getDatabase();
		Object input = treeViewer.getInput();
		if(input!=null && input instanceof List<?>){
			IWaveformDb db = ((List<IWaveformDb>)input).get(0);
			if(db==database) return; // do nothing if old and new daabase is the same
			((List<IWaveformDb>)input).get(0).removePropertyChangeListener(treeViewerPCL);
		}
		treeViewer.setInput(Arrays.asList(database.isLoaded()?new IWaveformDb[]{database}:new IWaveformDb[]{new LoadingWaveformDb()}));
		Object state=this.waveformViewerPart.retrieveDesignBrowerState();
		if(state!=null && state instanceof DBState) 
			((DBState)state).apply();
		else
			txTableViewer.setInput(null);
		// Set up the tree viewer
		database.addPropertyChangeListener(treeViewerPCL);
	} 

	/**
	 * Sets the selection.
	 *
	 * @param selection the selection
	 * @param partService the part service
	 */
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

	/**
	 * Update buttons.
	 */
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

	/**
	 * The Class WaveformAttributeFilter.
	 */
	public class WaveformAttributeFilter extends ViewerFilter {

		/** The search string. */
		private String searchString;

		/**
		 * Sets the search text.
		 *
		 * @param s the new search text
		 */
		public void setSearchText(String s) {
			this.searchString = ".*" + s + ".*";
		}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.viewers.ViewerFilter#select(org.eclipse.jface.viewers.Viewer, java.lang.Object, java.lang.Object)
		 */
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

	/**
	 * Gets the filtered children.
	 *
	 * @param viewer the viewer
	 * @return the filtered children
	 */
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

	/**
	 * Run command.
	 *
	 * @param handler the handler
	 * @param annotation the annotation
	 * @param where the where
	 * @param all the all
	 * @return the object
	 */
	protected Object runCommand(AddWaveformHandler handler, Class<? extends Annotation> annotation, String where, Boolean all) {
		ContextInjectionFactory.inject(handler, eclipseCtx);
		eclipseCtx.set(AddWaveformHandler.PARAM_WHERE_ID, where);
		eclipseCtx.set(AddWaveformHandler.PARAM_ALL_ID, all.toString());
		eclipseCtx.set(DesignBrowser.class, this);
		eclipseCtx.set(WaveformViewer.class, waveformViewerPart);
		Object result = ContextInjectionFactory.invoke(handler, annotation, eclipseCtx);
		return result;
	}

	/**
	 * Gets the filtered children.
	 *
	 * @return the filtered children
	 */
	public Object[] getFilteredChildren() {
		return getFilteredChildren(txTableViewer);
	}

	/**
	 * Gets the active waveform viewer part.
	 *
	 * @return the active waveform viewer part
	 */
	public WaveformViewer getActiveWaveformViewerPart() {
		return waveformViewerPart;
	}
	
	/**
	 * The Class DBState.
	 */
	class DBState {
		
		/**
		 * Instantiates a new DB state.
		 */
		public DBState() {
			this.expandedElements=treeViewer.getExpandedElements();
			this.treeSelection=treeViewer.getSelection();
			this.tableSelection=txTableViewer.getSelection();
		}
		
		/**
		 * Apply.
		 */
		public void apply() {
			treeViewer.setExpandedElements(expandedElements);
			treeViewer.setSelection(treeSelection, true);
			txTableViewer.setSelection(tableSelection, true);
			
		}

		/** The expanded elements. */
		private Object[] expandedElements;
		
		/** The tree selection. */
		private ISelection treeSelection;
		
		/** The table selection. */
		private ISelection tableSelection;
	}
};