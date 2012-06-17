package com.itjw.txviewer.graph.views.sections;

import java.util.HashMap;
import java.util.List;
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

import com.itjw.txviewer.database.ITrAttribute;
import com.itjw.txviewer.graph.data.ITransactionFacade;

public class AttributeProperty extends AbstractPropertySection implements ISelectionProvider {

	private final String[] titles = { "Location", "Name", "Type", "Value" };

	private ListenerList listeners = new ListenerList();

	private ITransactionFacade iTr;

	private ISelection currentSelection;

	private TreeViewer treeViewer;

	public AttributeProperty() {
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
		column1.setWidth(75);
		TreeColumn column2 = new TreeColumn(tree, SWT.RIGHT);
		column2.setAlignment(SWT.LEFT);
		column2.setText(titles[1]);
		column2.setWidth(150);
		TreeColumn column3 = new TreeColumn(tree, SWT.LEFT);
		tree.setLinesVisible(true);
		column3.setAlignment(SWT.LEFT);
		column3.setText(titles[2]);
		column3.setWidth(100);
		TreeColumn column4 = new TreeColumn(tree, SWT.RIGHT);
		column4.setAlignment(SWT.LEFT);
		column4.setText(titles[3]);
		column4.setWidth(150);

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
			TreeMap<String, List<ITrAttribute>> hier = new TreeMap<String, List<ITrAttribute>>();
			HashMap<ITrAttribute, String> parents = new HashMap<ITrAttribute, String>();

			@Override
			public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
				if (newInput instanceof String) {
					hier.clear();
					parents.clear();
					String location="Begin";
					List<ITrAttribute> txSet = iTr.getBeginAttrs();
					if (txSet != null && txSet.size() > 0) {
						hier.put(location, txSet);
						for (ITrAttribute tr : txSet)
							if (tr != null)
								parents.put(tr, location);
					}
					location="Recorded";
					txSet = iTr.getAttributes();
					if (txSet != null && txSet.size() > 0) {
						hier.put(location, txSet);
						for (ITrAttribute tr : txSet)
							if (tr != null)
								parents.put(tr, location);
					}
					location="End";
					txSet = iTr.getEndAttrs();
					if (txSet != null && txSet.size() > 0) {
						hier.put(location, txSet);
						for (ITrAttribute tr : txSet)
							if (tr != null)
								parents.put(tr, location);
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
				if (element instanceof ITrAttribute)
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
			public void dispose() {	}

			@Override
			public void addListener(ILabelProviderListener listener) { }

			@Override
			public String getColumnText(Object element, int columnIndex) {
				if (columnIndex == 0 && element instanceof String)
					return element.toString();
				else if(element instanceof ITrAttribute){
					ITrAttribute attr = (ITrAttribute)element;
					if (columnIndex == 1 )
						return attr.getName();
					else if (columnIndex == 2 )
						return attr.getType();
					else if (columnIndex == 3){
						if("UNSIGNED".equals(attr.getType()) &&
								(attr.getName().contains("addr")||attr.getName().contains("data")))
							return "0x"+Long.toHexString(Long.parseLong(attr.getValue().toString()));
						else 
							return attr.getValue().toString();
					}
				} 
				return null;
			}
			@Override
			public Image getColumnImage(Object element, int columnIndex) {
				return null;
			}
		});

		MenuManager menuMgr = new MenuManager("#PopUp"); //$NON-NLS-1$
		menuMgr.setRemoveAllWhenShown(true);
		menuMgr.addMenuListener(new IMenuListener() {
			public void menuAboutToShow(IMenuManager mgr) {
				ISelection selection = treeViewer.getSelection();
				if (selection instanceof IStructuredSelection) {
					System.out.println(((IStructuredSelection)selection).getFirstElement().toString());
				}
			}
		});
		Menu menu = menuMgr.createContextMenu(treeViewer.getControl());
		treeViewer.getControl().setMenu(menu);
		aTabbedPropertySheetPage.getSite().setSelectionProvider(this);
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
		treeViewer.setInput("All");
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

}
