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

import java.util.Vector;

import javax.annotation.PostConstruct;
import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.core.services.events.IEventBroker;
import org.eclipse.e4.ui.di.Focus;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;
import org.eclipse.jface.viewers.DelegatingStyledCellLabelProvider;
import org.eclipse.jface.viewers.DelegatingStyledCellLabelProvider.IStyledLabelProvider;
import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.viewers.StyledString;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.TreeViewerColumn;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerFilter;
import org.eclipse.jface.viewers.ViewerSorter;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.Tree;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxAttribute;
import com.minres.scviewer.database.ITxRelation;
import com.minres.scviewer.e4.application.provider.TxPropertiesLabelProvider;

/**
 * The Class TransactionDetails shows the details of a selected transaction. 
 */
public class TransactionDetails {

	/** The Constant COLUMN_FIRST. */
	// Column constants
	public static final int COLUMN_FIRST = 0;

	/** The Constant COLUMN_SECOND. */
	public static final int COLUMN_SECOND = 1;

	/** The Constant COLUMN_THIRD. */
	public static final int COLUMN_THIRD = 2;

	/** The event broker. */
	@Inject IEventBroker eventBroker;

	/** The selection service. */
	@Inject	ESelectionService selectionService;

	/** The name filter. */
	private Text nameFilter;

	/** The tree viewer. */
	private TreeViewer treeViewer;

	/** The col3. */
	private TreeViewerColumn col1, col2, col3;

	/** The attribute filter. */
	TxAttributeFilter attributeFilter;
	
	/** The view sorter. */
	TxAttributeViewerSorter viewSorter;

	/** The waveform viewer part. */
	private WaveformViewer waveformViewerPart;


	/**
	 * Creates the composite.
	 *
	 * @param parent the parent
	 */
	@PostConstruct
	public void createComposite(final Composite parent) {
		parent.setLayout(new GridLayout(1, false));

		nameFilter = new Text(parent, SWT.BORDER);
		nameFilter.setMessage("Enter text to filter");
		nameFilter.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e) {
				attributeFilter.setSearchText(((Text) e.widget).getText());
				treeViewer.refresh();
			}
		});
		nameFilter.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

		attributeFilter = new TxAttributeFilter();
		viewSorter = new TxAttributeViewerSorter();
		
		treeViewer = new TreeViewer(parent);
		treeViewer.setContentProvider(new TransactionTreeContentProvider());
		treeViewer.setLabelProvider(new TxPropertiesLabelProvider());
		treeViewer.getControl().setLayoutData(new GridData(GridData.FILL_BOTH));
		treeViewer.addFilter(attributeFilter);
		treeViewer.setSorter(viewSorter);
		treeViewer.setAutoExpandLevel(2);

		// Set up the table
		Tree tree = treeViewer.getTree();
		tree.setLayoutData(new GridData(GridData.FILL_BOTH));
		// Add the name column
		col1 = new TreeViewerColumn(treeViewer, SWT.NONE);
		col1.getColumn().setText("Name");
		col1.getColumn().setResizable(true);
		col1.setLabelProvider(new DelegatingStyledCellLabelProvider(new AttributeLabelProvider(AttributeLabelProvider.NAME)));
		col1.getColumn().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent event) {
				((TxAttributeViewerSorter) treeViewer.getSorter()).doSort(COLUMN_FIRST);
				treeViewer.refresh();
			}
		});
		// Add the type column
		col2 = new TreeViewerColumn(treeViewer, SWT.NONE);
		col2.getColumn().setText("Type");
		col2.getColumn().setResizable(true);
		col2.setLabelProvider(new DelegatingStyledCellLabelProvider(new AttributeLabelProvider(AttributeLabelProvider.TYPE)));
		col2.getColumn().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent event) {
				((TxAttributeViewerSorter) treeViewer.getSorter()).doSort(COLUMN_SECOND);
				treeViewer.refresh();
			}
		});
		// Add the value column
		col3 = new TreeViewerColumn(treeViewer, SWT.NONE);
		col3.getColumn().setText("Value");
		col3.getColumn().setResizable(true);
		col3.setLabelProvider(new DelegatingStyledCellLabelProvider(new AttributeLabelProvider(AttributeLabelProvider.VALUE)));
		col3.getColumn().addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent event) {
				((TxAttributeViewerSorter) treeViewer.getSorter()).doSort(COLUMN_SECOND);
				treeViewer.refresh();
			}
		});
		// Pack the columns
//		for (int i = 0, n = table.getColumnCount(); i < n; i++) {
//			table.getColumn(i).pack();
//		}

		// Turn on the header and the lines
		tree.setHeaderVisible(true);
		tree.setLinesVisible(true);
		
		treeViewer.addDoubleClickListener(new IDoubleClickListener(){

			@Override
			public void doubleClick(DoubleClickEvent event) {
				ISelection selection = treeViewer.getSelection();
				if(selection instanceof IStructuredSelection){
					IStructuredSelection structuredSelection = (IStructuredSelection) selection;
					Object selected = structuredSelection.getFirstElement();
					if(selected instanceof Object[]){
						Object[] selectedArray = (Object[]) selected;
						if(selectedArray.length==3 && selectedArray[2] instanceof ITx){
							waveformViewerPart.setSelection(new StructuredSelection(selectedArray[2]));
							treeViewer.setInput(selectedArray[2]);
						}
					}
				}
			}
			
		});
		parent.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e) {
				Tree table = treeViewer.getTree();
				Rectangle area = parent.getClientArea();
				Point preferredSize = table.computeSize(SWT.DEFAULT, SWT.DEFAULT);
				int width = area.width - 2*table.getBorderWidth();
				if (preferredSize.y > area.height + table.getHeaderHeight()) {
					// Subtract the scrollbar width from the total column width
					// if a vertical scrollbar will be required
					Point vBarSize = table.getVerticalBar().getSize();
					width -= vBarSize.x;
				}
				Point oldSize = table.getSize();
				if (oldSize.x > area.width) {
					// table is getting smaller so make the columns 
					// smaller first and then resize the table to
					// match the client area width
					col1.getColumn().setWidth(width/3);
					col2.getColumn().setWidth(width/4);
					col3.getColumn().setWidth(width - col1.getColumn().getWidth());
					table.setSize(area.width, area.height);
				} else {
					// table is getting bigger so make the table 
					// bigger first and then make the columns wider
					// to match the client area width
					table.setSize(area.width, area.height);
					col1.getColumn().setWidth(width/3);
					col2.getColumn().setWidth(width/4);
					col3.getColumn().setWidth(width - col1.getColumn().getWidth()- col2.getColumn().getWidth());
				}
			}
		});
	}

	/**
	 * Sets the focus.
	 */
	@Focus
	public void setFocus() {
		treeViewer.getTree().setFocus();
	}

	/**
	 * Gets the status event.
	 *
	 * @param part the part
	 * @return the status event
	 */
	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(WaveformViewer.ACTIVE_WAVEFORMVIEW) WaveformViewer part) {
		this.waveformViewerPart=part;
	}

	/**
	 * Sets the selection.
	 *
	 * @param selection the new selection
	 */
	@Inject
	public void setSelection(@Named(IServiceConstants.ACTIVE_SELECTION) @Optional IStructuredSelection selection){
		if(treeViewer!=null && selection!=null && !treeViewer.getTree().isDisposed()){
			if( selection instanceof IStructuredSelection) {
				Object object= ((IStructuredSelection)selection).getFirstElement();			
				if(object instanceof ITx){
					treeViewer.setInput(object);
				} else {
					treeViewer.setInput(null);
				}
			}
		}
	}

	/**
	 * Time to string.
	 *
	 * @param time the time
	 * @return the string
	 */
	String timeToString(Long time){
		return waveformViewerPart.getScaledTime(time);
	}

	/**
	 * Tx to string.
	 *
	 * @param tx the tx
	 * @return the string
	 */
	String txToString(ITx tx){
		StringBuilder sb = new StringBuilder();
		sb.append("tx#").append(tx.getId()).append("[").append(timeToString(tx.getBeginTime())).
			append(" - ").append(timeToString(tx.getEndTime())).append("]");
		return sb.toString();
	}
	
	/**
	 * The Class TxAttributeViewerSorter.
	 */
	class TxAttributeViewerSorter extends ViewerSorter {
		
		/** The Constant ASCENDING. */
		private static final int ASCENDING = 0;

		/** The Constant DESCENDING. */
		private static final int DESCENDING = 1;

		/** The column. */
		private int column;

		/** The direction. */
		private int direction;

		/**
		 * Does the sort. If it's a different column from the previous sort, do an
		 * ascending sort. If it's the same column as the last sort, toggle the sort
		 * direction.
		 *
		 * @param column the column
		 */
		public void doSort(int column) {
			if (column == this.column) {
				// Same column as last sort; toggle the direction
				direction = 1 - direction;
			} else {
				// New column; do an ascending sort
				this.column = column;
				direction = ASCENDING;
			}
		}

		/**
		 * Compares the object for sorting.
		 *
		 * @param viewer the viewer
		 * @param e1 the e1
		 * @param e2 the e2
		 * @return the int
		 */
		@SuppressWarnings("unchecked")
		public int compare(Viewer viewer, Object e1, Object e2) {
			int rc = 0;
			if(e1 instanceof ITxAttribute && e2 instanceof ITxAttribute){
				ITxAttribute p1 = (ITxAttribute) e1;
				ITxAttribute p2 = (ITxAttribute) e2;
				// Determine which column and do the appropriate sort
				switch (column) {
				case COLUMN_FIRST:
					rc = getComparator().compare(p1.getName(), p2.getName());
					break;
				case COLUMN_SECOND:
					rc = getComparator().compare(p1.getDataType().name(), p2.getDataType().name());
					break;
				case COLUMN_THIRD:
					rc = getComparator().compare(p1.getValue(), p2.getValue());
					break;
				}
				// If descending order, flip the direction
				if (direction == DESCENDING) rc = -rc;
			}
			return rc;
		}
	}

	/**
	 * The Class TxAttributeFilter.
	 */
	class TxAttributeFilter extends ViewerFilter {

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
			if(element instanceof ITxAttribute){
				ITxAttribute p = (ITxAttribute) element;
				if (p.getName().matches(searchString)) {
					return true;
				}
			} else if(element instanceof TreeNode)
				return true;
			return false;
		}
	}

	/**
	 * The Enum Type.
	 */
	enum Type {/** The props. */
PROPS, /** The attrs. */
 ATTRS, /** The in rel. */
 IN_REL, /** The out rel. */
 OUT_REL}

	/**
	 * The Class TreeNode.
	 */
	class TreeNode{
		
		/** The type. */
		public Type type;
		
		/** The element. */
		public ITx element;

		/**
		 * Instantiates a new tree node.
		 *
		 * @param element the element
		 * @param type the type
		 */
		public TreeNode(ITx element, Type type){
			this.element=element;
			this.type=type;
		}

		/* (non-Javadoc)
		 * @see java.lang.Object#toString()
		 */
		public String toString(){
			switch(type){
			case PROPS:      return "Properties";
			case ATTRS:	     return "Attributes";
			case IN_REL:     return "Incoming relations";
			case OUT_REL:    return "Outgoing relations";
			}
			return "";
		}
	}

	/**
	 * The Class TransactionTreeContentProvider.
	 */
	class TransactionTreeContentProvider implements ITreeContentProvider {

		/* (non-Javadoc)
		 * @see org.eclipse.jface.viewers.IContentProvider#dispose()
		 */
		@Override
		public void dispose() {	}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.viewers.IContentProvider#inputChanged(org.eclipse.jface.viewers.Viewer, java.lang.Object, java.lang.Object)
		 */
		@Override
		public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
		}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.viewers.ITreeContentProvider#getElements(java.lang.Object)
		 */
		@Override
		public Object[] getElements(Object element) {
			return new Object[]{
					new TreeNode((ITx)element, Type.PROPS),  
					new TreeNode((ITx)element, Type.ATTRS),  
					new TreeNode((ITx)element, Type.IN_REL),
					new TreeNode((ITx)element, Type.OUT_REL)
			};
		}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.viewers.ITreeContentProvider#getChildren(java.lang.Object)
		 */
		@Override
		public Object[] getChildren(Object element) {
			if(element instanceof TreeNode){
				TreeNode propertyHolder=(TreeNode) element;
				if(propertyHolder.type == Type.PROPS){
					return new Object[][]{
						{"Name", "String", propertyHolder.element.getStream().getFullName()},
						{"Type", "String", propertyHolder.element.getGenerator().getName()},
						{"Start time", "Time", timeToString(propertyHolder.element.getBeginTime())},
						{"End time", "Time", timeToString(propertyHolder.element.getEndTime())}
					};
				}else if(propertyHolder.type == Type.ATTRS)
					return propertyHolder.element.getAttributes().toArray();
				else if(propertyHolder.type == Type.IN_REL){
					Vector<Object[] > res = new Vector<>();
					for(ITxRelation rel:propertyHolder.element.getIncomingRelations()){
						res.add(new Object[]{
								rel.getRelationType(), 
								rel.getSource().getGenerator().getName(), 
								rel.getSource()});
					}
					return res.toArray();
				} else if(propertyHolder.type == Type.OUT_REL){
					Vector<Object[] > res = new Vector<>();
					for(ITxRelation rel:propertyHolder.element.getOutgoingRelations()){
						res.add(new Object[]{
								rel.getRelationType(), 
								rel.getTarget().getGenerator().getName(), 
								rel.getTarget()});
					}
					return res.toArray();
				}
			}
			return null;
		}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.viewers.ITreeContentProvider#getParent(java.lang.Object)
		 */
		@Override
		public Object getParent(Object element) {
			return null;
		}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.viewers.ITreeContentProvider#hasChildren(java.lang.Object)
		 */
		@Override
		public boolean hasChildren(Object element) {
			return getChildren(element)!=null;
		}

	}

	/**
	 * The Class AttributeLabelProvider.
	 */
	class AttributeLabelProvider extends LabelProvider implements IStyledLabelProvider {
		
		/** The field. */
		final int field;
		
		/** The Constant NAME. */
		public static final int NAME=0;
		
		/** The Constant TYPE. */
		public static final int TYPE=1;
		
		/** The Constant VALUE. */
		public static final int VALUE=2;

		/**
		 * Instantiates a new attribute label provider.
		 *
		 * @param field the field
		 */
		public  AttributeLabelProvider(int field) {
			this.field=field;
		}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.viewers.DelegatingStyledCellLabelProvider.IStyledLabelProvider#getStyledText(java.lang.Object)
		 */
		@Override
		public StyledString getStyledText(Object element) {
			switch(field){
			case NAME:
				if (element instanceof ITxAttribute) {
					ITxAttribute attribute = (ITxAttribute) element;
					return new StyledString(attribute.getName());
				}else if (element instanceof ITxRelation) {
					return new StyledString("Relation");
				}else if(element instanceof Object[]){
					Object[] elements = (Object[]) element;
					return new StyledString(elements[field].toString());
				} else 
					return new StyledString(element.toString());
			case TYPE:
				if (element instanceof ITxAttribute) {
					ITxAttribute attribute = (ITxAttribute) element;
					return new StyledString(attribute.getDataType().toString());
				}else if(element instanceof Object[]){
					Object[] elements = (Object[]) element;
					return new StyledString(elements[field].toString());
				}else 
					return new StyledString("");					
			default:
				if (element instanceof ITxAttribute) {
					ITxAttribute attribute = (ITxAttribute) element;
					return new StyledString(attribute.getValue().toString());
				}else if(element instanceof Object[]){
					Object[] elements = (Object[]) element;
					return new StyledString(elements[field].toString());
				} else if(element instanceof ITx){
					return new StyledString(txToString((ITx) element));
				}else 
					return new StyledString("");					
			}
		}
	}
}

