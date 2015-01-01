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
package com.itjw.txviewer.ui.views;

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
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.ISelectionListener;
import org.eclipse.ui.ISharedImages;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.actions.ActionFactory;
import org.eclipse.ui.part.IPageSite;
import org.eclipse.ui.views.contentoutline.ContentOutline;
import org.eclipse.ui.views.contentoutline.ContentOutlinePage;

import com.itjw.txviewer.database.ITrHierNode;
import com.itjw.txviewer.database.ITrStream;
import com.itjw.txviewer.database.ITransaction;
import com.itjw.txviewer.ui.TxEditorPart;
import com.itjw.txviewer.ui.views.provider.TxDbTreeContentProvider;
import com.itjw.txviewer.ui.views.provider.TxDbTreeLabelProvider;

/**
 * Creates an outline pagebook for this editor.
 */
public class TxOutlinePage extends ContentOutlinePage implements  ISelectionListener, ISelectionProvider {

	public static final int ADD_TO_WAVE = 0;
	public static final int ADD_ALL_TO_WAVE = 1;
	public static final int REMOVE_FROM_WAVE = 2;
	public static final int REMOVE_ALL_FROM_WAVE = 3;

	private TxEditorPart editor;

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
		TreeViewer contentOutlineViewer = getTreeViewer();
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
		getSite().registerContextMenu("com.itjw.txviewer.ui.outline.contextmenu", menuMgr, contentOutlineViewer);
		// add me as selection listener
		getSite().getPage().addSelectionListener((ISelectionListener) this);
		//getSite().getPage().addSelectionListener("SampleViewId",(ISelectionListener)this);
		getSite().setSelectionProvider(this);
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
		IActionBars bars = pageSite.getActionBars();
	}

	private void fillContextMenu(IMenuManager menuMgr) {
		// initalize the context menu
		getTreeViewer().getSelection();
		ISelection selection = getTreeViewer().getSelection();
		if(selection instanceof IStructuredSelection){
			IStructuredSelection sel = (IStructuredSelection) selection;
			Object obj = sel.getFirstElement();
			menuMgr.add(makeStreamAction("Add to Wave", ISharedImages.IMG_OBJ_ADD, sel, obj instanceof ITrStream, false));
			menuMgr.add(makeStreamAction("Add all to Wave", ISharedImages.IMG_OBJ_ADD, sel, true, false));
			menuMgr.add(makeStreamAction("Remove from Wave", ISharedImages.IMG_TOOL_DELETE, sel, obj instanceof ITrStream,true));
			menuMgr.add(makeStreamAction("Remove all from Wave", ISharedImages.IMG_TOOL_DELETE, sel, true, true));	
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
             if (tmp instanceof ITrHierNode) {
            	 fireSelectionChanged(new StructuredSelection((ITrHierNode) tmp));
             }
         }
     }

	private Action makeStreamAction(String text, String imgDescriptor, final IStructuredSelection selection, boolean enabled, final boolean remove) {
		Action action = new Action() {
			public void run() {
				if(selection!=null)
					for(Object obj :selection.toArray()){
						if(obj instanceof ITrStream){
							if(remove)
								editor.removeStreamFromList((ITrStream) obj);
							else
								editor.addStreamToList((ITrStream) obj);
						} else if(obj instanceof ITrHierNode){
							LinkedList<ITrHierNode> queue = new LinkedList<ITrHierNode>();
							LinkedList<ITrStream> streams = new LinkedList<ITrStream>();
							queue.add((ITrHierNode)obj);
							while(queue.size()>0){
								ITrHierNode n = queue.poll();
								if(n instanceof ITrStream) streams.add((ITrStream) n);
								queue.addAll(n.getChildNodes());
							}
							if(remove)
								editor.removeStreamsFromList(streams.toArray(new ITrStream[]{}));
							else
								editor.addStreamsToList(streams.toArray(new ITrStream[]{}));
						}
					}
			}
		};
		action.setText(text);
		action.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(imgDescriptor));
		if(selection.getFirstElement() instanceof ITrStream && editor.getStreamList().contains(selection.getFirstElement()))
			action.setEnabled(false);
		else
			action.setEnabled(true);
		return action;
	}

}
