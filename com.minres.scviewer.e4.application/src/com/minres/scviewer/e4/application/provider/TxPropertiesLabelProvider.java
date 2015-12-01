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
package com.minres.scviewer.e4.application.provider;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.swt.graphics.Image;

import com.minres.scviewer.database.ITxAttribute;
import com.minres.scviewer.e4.application.parts.TransactionDetails;

/**
 * The Class TxPropertiesLabelProvider providing the labels of a property to the respective viewer.
 */
public class TxPropertiesLabelProvider implements ITableLabelProvider {

	/** The listeners. */
	private List<ILabelProviderListener> listeners = new ArrayList<ILabelProviderListener>();
		
	/**
	 * Instantiates a new tx properties label provider.
	 */
	public TxPropertiesLabelProvider() {
		super();
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#dispose()
	 */
	@Override
	public void dispose() {
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#addListener(org.eclipse.jface.viewers.ILabelProviderListener)
	 */
	@Override
	public void addListener(ILabelProviderListener listener) {
		  listeners.add(listener);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#removeListener(org.eclipse.jface.viewers.ILabelProviderListener)
	 */
	@Override
	public void removeListener(ILabelProviderListener listener) {
		  listeners.remove(listener);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#isLabelProperty(java.lang.Object, java.lang.String)
	 */
	@Override
	public boolean isLabelProperty(Object element, String property) {
		  return false;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.ITableLabelProvider#getColumnImage(java.lang.Object, int)
	 */
	@Override
	public Image getColumnImage(Object element, int columnIndex) {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.ITableLabelProvider#getColumnText(java.lang.Object, int)
	 */
	@Override
	public String getColumnText(Object element, int columnIndex) {
		ITxAttribute attribute = (ITxAttribute) element;
	    String text = "";
	    switch (columnIndex) {
	    case TransactionDetails.COLUMN_FIRST:
	      text = attribute.getName();
	      break;
	    case TransactionDetails.COLUMN_SECOND:
	      text = attribute.getValue().toString();
	      break;
	    }
	    return text;
	}

}


