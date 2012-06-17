package com.itjw.txviewer.graph.views.sections;

import java.util.HashMap;
import java.util.Set;
import java.util.TreeMap;

import org.eclipse.core.runtime.Assert;
import org.eclipse.core.runtime.ListenerList;
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
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.views.properties.tabbed.AbstractPropertySection;
import org.eclipse.ui.views.properties.tabbed.TabbedPropertySheetPage;

import com.itjw.txviewer.database.ITransaction;
import com.itjw.txviewer.database.RelationType;
import com.itjw.txviewer.graph.TransactionSelection;
import com.itjw.txviewer.graph.actions.TxActionFactory;
import com.itjw.txviewer.graph.data.ITransactionFacade;

public class RelatedProperty extends AbstractPropertySection implements ISelectionProvider, ISelectionChangedListener {

	private final String[] titles = { "Relation", "Tx Id" };

	private ListenerList listeners = new ListenerList();

	private ITransactionFacade iTr;

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
		TreeColumn column2 = new TreeColumn(tree, SWT.RIGHT);
		column2.setAlignment(SWT.LEFT);
		column2.setText(titles[1]);
		column2.setWidth(150);

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
			TreeMap<RelationType, Set<ITransaction>> hier = new TreeMap<RelationType, Set<ITransaction>>();
			HashMap<ITransaction, RelationType> parents = new HashMap<ITransaction, RelationType>();

			@Override
			public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
				if (newInput instanceof RelationType[]) {
					hier.clear();
					parents.clear();
					for (RelationType rel : (RelationType[]) newInput) {
						Set<ITransaction> txSet = iTr.getNextInRelationship(rel);
						if (txSet != null && txSet.size() > 0) {
							boolean hasChilds=false;
							for (ITransaction tr : txSet)
								if (tr != null){
									parents.put(tr, rel);
									hasChilds=true;
								}
							if(hasChilds) hier.put(rel, txSet);
						}
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
				if (element instanceof ITransaction)
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
				if (parentElement instanceof RelationType)
					return hier.get((RelationType) parentElement).toArray();
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
				if (columnIndex == 0 && element instanceof RelationType)
					return element.toString();
				else if (columnIndex == 1 && element instanceof ITransaction)
					return ((ITransaction) element).getId().toString();
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
				fillContextMenu(mgr);
			}
		});
		Menu menu = menuMgr.createContextMenu(treeViewer.getControl());
		treeViewer.getControl().setMenu(menu);
		aTabbedPropertySheetPage.getSite().setSelectionProvider(this);
		getPart().getSite().setSelectionProvider(this);

	}

	private void fillContextMenu(IMenuManager menuMgr) {
		// initalize the context menu
		ISelection selection = treeViewer.getSelection();
		if (selection instanceof IStructuredSelection) {
			Object obj = ((IStructuredSelection) selection).getFirstElement();
			menuMgr.add(TxActionFactory.getAction(TxActionFactory.JUMP_TO_TX,  obj instanceof ITransaction));
		}
	}

	public void setInput(IWorkbenchPart part, ISelection selection) {
		super.setInput(part, selection);
		currentSelection = null;
		Assert.isTrue(selection instanceof IStructuredSelection);
		Object input = ((IStructuredSelection) selection).getFirstElement();
		Assert.isTrue(input instanceof ITransactionFacade);
		iTr = (ITransactionFacade) input;
	}

	public void refresh() {
		treeViewer.setInput(RelationType.values());
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
		ISelection selection = event.getSelection();
		if(selection instanceof IStructuredSelection){
			IStructuredSelection treeSelection =(IStructuredSelection)selection;
			Object elem = treeSelection.getFirstElement();
			if(elem  instanceof ITransactionFacade){
				currentSelection = new TransactionSelection((ITransactionFacade)elem);
			} else if(elem  instanceof ITransaction){
				currentSelection = new TransactionSelection(new ITransactionFacade((ITransaction)elem));
			}
		}
	}

}
