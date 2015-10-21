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
package com.minres.scviewer.ui.views;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.LinkedList;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.ui.ISelectionListener;
import org.eclipse.ui.ISharedImages;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.part.IPageSite;
import org.eclipse.ui.views.contentoutline.ContentOutlinePage;

import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.ui.TxEditorPart;
import com.minres.scviewer.ui.views.provider.TxDbTreeContentProvider;
import com.minres.scviewer.ui.views.provider.TxDbTreeLabelProvider;

/**
 * Creates an outline pagebook for this editor.
 */
public class TxOutlinePage extends ContentOutlinePage implements  ISelectionListener, ISelectionProvider, PropertyChangeListener {

	public static final int ADD_TO_WAVE = 0;
	public static final int ADD_ALL_TO_WAVE = 1;
	public static final int REMOVE_FROM_WAVE = 2;
	public static final int REMOVE_ALL_FROM_WAVE = 3;

	private TxEditorPart editor;
	TreeViewer contentOutlineViewer ;
	
	public TxOutlinePage(TxEditorPart editor) {
		this.editor = editor;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * org.eclipse.ui.part.IPage#createControl(org.eclipse.swt.widgets.Composite
	 * )
	 */
	public void createControl(Composite parent) {
		super.createControl(parent);
		contentOutlineViewer = getTreeViewer();
		contentOutlineViewer.addSelectionChangedListener(this);
		// Set up the tree viewer
		contentOutlineViewer.setContentProvider(new TxDbTreeContentProvider());
		contentOutlineViewer.setLabelProvider(new TxDbTreeLabelProvider());
		contentOutlineViewer.setInput(editor.getDatabase());
		// initialize context menu depending on the the selectec item
		MenuManager menuMgr = new MenuManager();
		menuMgr.setRemoveAllWhenShown(true);
		menuMgr.addMenuListener(new IMenuListener() {
			public void menuAboutToShow(IMenuManager manager) {
				fillContextMenu(manager);
			}
		});
		Menu menu = menuMgr.createContextMenu(contentOutlineViewer.getControl());
		contentOutlineViewer.getTree().setMenu(menu);
		getSite().registerContextMenu("com.minres.scviewer.ui.outline.contextmenu", menuMgr, contentOutlineViewer);
		// add me as selection listener
		getSite().getPage().addSelectionListener((ISelectionListener) this);
		//getSite().getPage().addSelectionListener("SampleViewId",(ISelectionListener)this);
		getSite().setSelectionProvider(this);
		editor.getDatabase().addPropertyChangeListener(this);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.IPage#dispose()
	 */
	public void dispose() {
		// dispose
		super.dispose();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.IPage#getControl()
	 */
	public Control getControl() {
		return getTreeViewer().getControl();
	}

	/**
	 * @see org.eclipse.ui.part.IPageBookViewPage#init(org.eclipse.ui.part.IPageSite)
	 */
	public void init(IPageSite pageSite) {
		super.init(pageSite);
//		IActionBars bars = pageSite.getActionBars();
	}

	private void fillContextMenu(IMenuManager menuMgr) {
		// initalize the context menu
		getTreeViewer().getSelection();
		ISelection selection = getTreeViewer().getSelection();
		if(selection instanceof IStructuredSelection){
			IStructuredSelection sel = (IStructuredSelection) selection;
			Object obj = sel.getFirstElement();
			menuMgr.add(makeStreamAction("Add to Wave", ISharedImages.IMG_OBJ_ADD, sel, obj instanceof IWaveform, false));
			menuMgr.add(makeStreamAction("Add all to Wave", ISharedImages.IMG_OBJ_ADD, sel, true, false));
//			menuMgr.add(makeStreamAction("Remove from Wave", ISharedImages.IMG_TOOL_DELETE, sel, obj instanceof IWaveform, true));
//			menuMgr.add(makeStreamAction("Remove all from Wave", ISharedImages.IMG_TOOL_DELETE, sel, true, true));	
		}
	}

	
	//ISelectionListener methods
	@Override
	public void selectionChanged(IWorkbenchPart part, ISelection selection) {
//		if(!(part instanceof ContentOutline) && selection instanceof IStructuredSelection){
//			if(((IStructuredSelection)selection).getFirstElement() instanceof ITransaction){
//				System.out.println("Transaction with id "+((ITransaction)((IStructuredSelection)selection).getFirstElement()).getId() +" selected");			
//			} else if(((IStructuredSelection)selection).getFirstElement() != null)
//				System.out.println("Something else selected");			
//		}
	}
	
    /**
     * Returns the current selection for this provider.
     * 
     * @return the current selection
     */
    public ISelection getSelection() {
        if (getTreeViewer() == null) {
			return StructuredSelection.EMPTY;
		}
        return getTreeViewer().getSelection();
    }

	public void setSelection(ISelection selection){
	    if (getTreeViewer() != null) {
	    	getTreeViewer().setSelection(selection);
		}
	}
    /**
      * @see org.eclipse.jface.viewers.ISelectionProvider#selectionChanged(SelectionChangedEvent)
      */
     public void selectionChanged(SelectionChangedEvent anEvent) {
    	 // translate the tree selection
         ISelection selection = anEvent.getSelection();
         if (!selection.isEmpty()) {
             Object tmp = ((IStructuredSelection) selection).getFirstElement();
             if (tmp instanceof IHierNode) {
            	 fireSelectionChanged(new StructuredSelection((IHierNode) tmp));
             }
         }
     }

	private Action makeStreamAction(String text, String imgDescriptor, final IStructuredSelection selection, boolean enabled, final boolean remove) {
		Action action = new Action() {
			@SuppressWarnings("unchecked")
			public void run() {
				if(selection!=null)
					for(Object obj :selection.toArray()){
						if(obj instanceof IWaveform){
							if(remove)
								editor.removeStreamFromList((IWaveform<? extends IWaveformEvent>) obj);
							else
								editor.addStreamToList((IWaveform<? extends IWaveformEvent>) obj);
						} else if(obj instanceof IHierNode){
							LinkedList<IHierNode> queue = new LinkedList<IHierNode>();
							LinkedList<IWaveform<? extends IWaveformEvent>> streams = new LinkedList<IWaveform<? extends IWaveformEvent>>();
							queue.add((IHierNode)obj);
							while(queue.size()>0){
								IHierNode n = queue.poll();
								if(n instanceof IWaveform) streams.add((IWaveform<? extends IWaveformEvent>) n);
								queue.addAll(n.getChildNodes());
							}
							if(remove)
								editor.removeStreamsFromList(streams.toArray(new IWaveform[]{}));
							else
								editor.addStreamsToList(streams.toArray(new IWaveform[]{}));
						}
					}
			}
		};
		action.setText(text);
		action.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(imgDescriptor));
		if(selection.getFirstElement() instanceof IWaveform && editor.getStreamList().contains(selection.getFirstElement()))
			action.setEnabled(false);
		else
			action.setEnabled(true);
		return action;
	}

	@Override
	public void propertyChange(PropertyChangeEvent evt) {
		if("CHILDS".equals(evt.getPropertyName())) {
			contentOutlineViewer.refresh();
		}
	}

}
