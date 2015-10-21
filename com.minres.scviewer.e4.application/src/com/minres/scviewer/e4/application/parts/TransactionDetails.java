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
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.ESelectionService;
import org.eclipse.jface.viewers.TableViewer;
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
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.Text;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxAttribute;
import com.minres.scviewer.e4.application.provider.TxPropertiesContentProvider;
import com.minres.scviewer.e4.application.provider.TxPropertiesLabelProvider;

public class TransactionDetails {

	// Column constants
	public static final int COLUMN_FIRST = 0;

	public static final int COLUMN_SECOND = 1;

	@Inject IEventBroker eventBroker;

	@Inject	ESelectionService selectionService;

	private Text nameFilter;
	private TableViewer txTableViewer;
	private TableColumn col1, col2;
	TxAttributeFilter attributeFilter;
	
	
	@PostConstruct
	public void createComposite(final Composite parent) {
		parent.setLayout(new GridLayout(1, false));

		nameFilter = new Text(parent, SWT.BORDER);
		nameFilter.setMessage("Enter text to filter");
		nameFilter.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e) {
				attributeFilter.setSearchText(((Text) e.widget).getText());
				txTableViewer.refresh();
			}
		});
		nameFilter.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

		attributeFilter = new TxAttributeFilter();
		
		txTableViewer = new TableViewer(parent);
		txTableViewer.setContentProvider(new TxPropertiesContentProvider());
		txTableViewer.setLabelProvider(new TxPropertiesLabelProvider());
		txTableViewer.getTable().setLayoutData(new GridData(GridData.FILL_BOTH));
		txTableViewer.addFilter(attributeFilter);
		
		// Set up the table
		Table table = txTableViewer.getTable();
		table.setLayoutData(new GridData(GridData.FILL_BOTH));

		// Add the first name column
		col1 = new TableColumn(table, SWT.LEFT);
		col1.setText("Name");
		col1.setResizable(true);
		col1.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent event) {
				((TxAttributeViewerSorter) txTableViewer.getSorter()).doSort(COLUMN_FIRST);
				txTableViewer.refresh();
			}
		});

		// Add the last name column
		col2 = new TableColumn(table, SWT.LEFT);
		col2.setText("Value");
		col2.setResizable(true);
		col2.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent event) {
				((TxAttributeViewerSorter) txTableViewer.getSorter()).doSort(COLUMN_SECOND);
				txTableViewer.refresh();
			}
		});

		// Pack the columns
		for (int i = 0, n = table.getColumnCount(); i < n; i++) {
			table.getColumn(i).pack();
		}

		// Turn on the header and the lines
		table.setHeaderVisible(true);
		table.setLinesVisible(true);
		
		parent.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e) {
				Table table = txTableViewer.getTable();
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
					col1.setWidth(width/3);
					col2.setWidth(width - col1.getWidth());
					table.setSize(area.width, area.height);
				} else {
					// table is getting bigger so make the table 
					// bigger first and then make the columns wider
					// to match the client area width
					table.setSize(area.width, area.height);
					col1.setWidth(width/3);
					col2.setWidth(width - col1.getWidth());
				}
			}
		});
	}

	@Focus
	public void setFocus() {
		txTableViewer.getTable().setFocus();
	}

	@Inject
	public void setSelection(@Named(IServiceConstants.ACTIVE_SELECTION) @Optional Object object){
		if(txTableViewer!=null && !txTableViewer.getTable().isDisposed())
			if(object instanceof ITx){
				txTableViewer.setInput(object);
			} else {
				txTableViewer.setInput(null);
			}
	}

	class TxAttributeViewerSorter extends ViewerSorter {
		private static final int ASCENDING = 0;

		private static final int DESCENDING = 1;

		private int column;

		private int direction;

		/**
		 * Does the sort. If it's a different column from the previous sort, do an
		 * ascending sort. If it's the same column as the last sort, toggle the sort
		 * direction.
		 * 
		 * @param column
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
		 * Compares the object for sorting
		 */
		@SuppressWarnings("unchecked")
		public int compare(Viewer viewer, Object e1, Object e2) {
			int rc = 0;
			ITxAttribute p1 = (ITxAttribute) e1;
			ITxAttribute p2 = (ITxAttribute) e2;

			// Determine which column and do the appropriate sort
			switch (column) {
			case COLUMN_FIRST:
				rc = getComparator().compare(p1.getName(), p2.getName());
				break;
			case COLUMN_SECOND:
				rc = getComparator().compare(p1.getValue(), p2.getValue());
				break;
			}

			// If descending order, flip the direction
			if (direction == DESCENDING)
				rc = -rc;

			return rc;
		}
	}

	public class TxAttributeFilter extends ViewerFilter {

		  private String searchString;

		  public void setSearchText(String s) {
		    this.searchString = ".*" + s + ".*";
		  }

		  @Override
		  public boolean select(Viewer viewer, Object parentElement, Object element) {
		    if (searchString == null || searchString.length() == 0) {
		      return true;
		    }
		    ITxAttribute p = (ITxAttribute) element;
		    if (p.getName().matches(searchString)) {
		      return true;
		    }
		    return false;
		  }
		}
}