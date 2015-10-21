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
package com.minres.scviewer.ui.views.sections;

import java.util.Collection;
import java.util.HashMap;
import java.util.TreeMap;

import org.eclipse.core.runtime.Assert;
import org.eclipse.core.runtime.ListenerList;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Tree;
import org.eclipse.swt.widgets.TreeColumn;
import org.eclipse.ui.IEditorReference;
import org.eclipse.ui.ISharedImages;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.views.properties.tabbed.AbstractPropertySection;
import org.eclipse.ui.views.properties.tabbed.TabbedPropertySheetPage;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxRelation;

public class RelatedProperty extends AbstractPropertySection implements ISelectionProvider, ISelectionChangedListener {

	private final String[] titles = { "Relation type", "Relation Name", "Tx Id" };

	private ListenerList listeners = new ListenerList();

	private ITx iTr;

	private ISelection currentSelection;

	private TreeViewer treeViewer;

	public RelatedProperty() {
	}

	public void createControls(Composite parent, TabbedPropertySheetPage aTabbedPropertySheetPage) {
		super.createControls(parent, aTabbedPropertySheetPage);
		Composite composite = getWidgetFactory().createFlatFormComposite(parent);
		Tree tree = new Tree(composite, SWT.BORDER | SWT.FULL_SELECTION);
		tree.setHeaderVisible(true);
		treeViewer = new TreeViewer(tree);

		TreeColumn column1 = new TreeColumn(tree, SWT.LEFT);
		tree.setLinesVisible(true);
		column1.setAlignment(SWT.LEFT);
		column1.setText(titles[0]);
		column1.setWidth(150);
		TreeColumn column2 = new TreeColumn(tree, SWT.LEFT);
		column2.setAlignment(SWT.LEFT);
		column2.setText(titles[1]);
		column2.setWidth(150);
		TreeColumn column3 = new TreeColumn(tree, SWT.RIGHT);
		column3.setAlignment(SWT.LEFT);
		column3.setText(titles[2]);
		column3.setWidth(150);

		Object layoutData = parent.getLayoutData();
		if (layoutData instanceof GridData) {
			GridData gridData = (GridData) layoutData;
			gridData.grabExcessVerticalSpace = true;
			gridData.verticalAlignment = SWT.FILL;
		}

		FormData formData = new FormData();
		formData.left = new FormAttachment(0);
		formData.top = new FormAttachment(0);
		formData.right = new FormAttachment(100);
		formData.bottom = new FormAttachment(100);
		tree.setLayoutData(formData);

		treeViewer.setAutoExpandLevel(2);
		treeViewer.setContentProvider(new ITreeContentProvider() {
			TreeMap<String, Collection<ITxRelation>> hier = new TreeMap<String, Collection<ITxRelation>>();
			HashMap<ITxRelation, String> parents = new HashMap<ITxRelation, String>();

			@Override
			public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
				if (newInput instanceof ITx) {
					hier.clear();
					parents.clear();
					String relName = "incoming";
					Collection<ITxRelation> relSet = ((ITx)newInput).getIncomingRelations();
					if (relSet != null && relSet.size() > 0) {
						hier.put(relName, relSet);
						for (ITxRelation rel : relSet)
							parents.put(rel, relName);
					}
					relName = "outgoing";
					relSet = ((ITx)newInput).getOutgoingRelations();
					if (relSet != null && relSet.size() > 0) {
						hier.put(relName, relSet);
						for (ITxRelation rel : relSet)
							parents.put(rel, relName);
					}
				}
			}

			@Override
			public void dispose() {	}

			@Override
			public boolean hasChildren(Object element) {
				Object[] childs = getChildren(element);
				return childs != null && childs.length > 0;
			}

			@Override
			public Object getParent(Object element) {
				if (element instanceof ITx)
					return parents.get(element);
				else
					return null;
			}

			@Override
			public Object[] getElements(Object inputElement) {
				return hier.keySet().toArray();
			}

			@Override
			public Object[] getChildren(Object parentElement) {
				if (parentElement instanceof String)
					return hier.get((String) parentElement).toArray();
				else
					return null;
			}
		});
		treeViewer.setLabelProvider(new ITableLabelProvider() {
			@Override
			public void removeListener(ILabelProviderListener listener) { }

			@Override
			public boolean isLabelProperty(Object element, String property) {
				return false;
			}

			@Override
			public void dispose() { }

			@Override
			public void addListener(ILabelProviderListener listener) {	}

			@Override
			public String getColumnText(Object element, int columnIndex) {
				if (columnIndex == 0 && element instanceof String)
					return element.toString();
				else if (columnIndex == 1 && element instanceof ITxRelation)
					return ((ITxRelation) element).getRelationType().getName();
				else if (columnIndex == 2 && element instanceof ITxRelation){
					ITxRelation rel = (ITxRelation) element;
					if(rel.getTarget()==iTr) 
						return ((ITxRelation) element).getSource().getId().toString();
					else
						return ((ITxRelation) element).getTarget().getId().toString();
				}
				else
					return null;
			}

			@Override
			public Image getColumnImage(Object element, int columnIndex) {
				return null;
			}
		});
		treeViewer.addSelectionChangedListener(this);
		MenuManager menuMgr = new MenuManager("#PopUp"); //$NON-NLS-1$
		menuMgr.setRemoveAllWhenShown(true);
		menuMgr.addMenuListener(new IMenuListener() {
			public void menuAboutToShow(IMenuManager mgr) {
				ISelection selection = treeViewer.getSelection();
				if (selection instanceof IStructuredSelection) {
					Object obj = ((IStructuredSelection) selection).getFirstElement();
					mgr.add(makeTransactionAction(obj, iTr));
				}
			}
		});
		Menu menu = menuMgr.createContextMenu(treeViewer.getControl());
		treeViewer.getControl().setMenu(menu);
//		aTabbedPropertySheetPage.getSite().setSelectionProvider(this);
//		if(getPart()!=null){
//			getPart().getSite().setSelectionProvider(this);
//		}
	}

	private Action makeTransactionAction(final Object obj, final ITx transaction) {
		Action action = new Action() {
			public void run() {
				if(obj instanceof ITxRelation){
					IWorkbenchPage page = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
					ITx targetTransaction  = ((ITxRelation)obj).getSource()==transaction?
							((ITxRelation)obj).getTarget():((ITxRelation)obj).getSource();
					for(IEditorReference editorRef: page.getEditorReferences()){
						IWorkbenchPart part =editorRef.getPart(false);
						if(editorRef.getPage().isPartVisible(part)){
							part.getSite().getSelectionProvider().setSelection(new StructuredSelection(targetTransaction));
							part.setFocus();
						}
					}
				}	
			}
		};
		action.setText("Jump to Transaction");
		action.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(ISharedImages.IMG_OBJ_ADD));
		action.setEnabled(true);
		return action;
	}

	public void setInput(IWorkbenchPart part, ISelection selection) {
		super.setInput(part, selection);
		currentSelection = null;
		Assert.isTrue(selection instanceof IStructuredSelection);
		Object input = ((IStructuredSelection) selection).getFirstElement();
		Assert.isTrue(input instanceof ITx);
		iTr = (ITx) input;
	}

	public void refresh() {
		treeViewer.setInput(iTr);
	}

	public void aboutToBeShown() {
		treeViewer.expandAll();
	}

	@Override
	public void addSelectionChangedListener(ISelectionChangedListener listener) {
		listeners.add(listener);
	}

	@Override
	public void removeSelectionChangedListener(ISelectionChangedListener listener) {
		listeners.remove(listener);
	}

	public ISelection getSelection() {
		return currentSelection != null ? currentSelection : super.getSelection();
	}

	@Override
	public void setSelection(ISelection selection) {
		currentSelection = selection;
		Object[] list = listeners.getListeners();
		for (int i = 0; i < list.length; i++) {
			((ISelectionChangedListener) list[i]).selectionChanged(new SelectionChangedEvent(this, selection));
		}
	}

	@Override
	public void selectionChanged(SelectionChangedEvent event) {
//		ISelection selection = event.getSelection();
//		if(selection instanceof IStructuredSelection){
//			IStructuredSelection treeSelection =(IStructuredSelection)selection;
//			Object elem = treeSelection.getFirstElement();
//			if(elem  instanceof ITransaction){
//				currentSelection = new StructuredSelection(elem);
//			}
//		}
	}

}
