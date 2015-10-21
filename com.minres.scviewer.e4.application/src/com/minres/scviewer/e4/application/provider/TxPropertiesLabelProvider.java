/*******************************************************************************
 * Copyright (c) 2014, 2015 MINRES Technologies GmbH and others.
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

public class TxPropertiesLabelProvider implements ITableLabelProvider {

	private List<ILabelProviderListener> listeners = new ArrayList<ILabelProviderListener>();
		
	public TxPropertiesLabelProvider() {
		super();
	}

	@Override
	public void dispose() {
	}

	@Override
	public void addListener(ILabelProviderListener listener) {
		  listeners.add(listener);
	}

	@Override
	public void removeListener(ILabelProviderListener listener) {
		  listeners.remove(listener);
	}

	@Override
	public boolean isLabelProperty(Object element, String property) {
		  return false;
	}

	@Override
	public Image getColumnImage(Object element, int columnIndex) {
		return null;
	}

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


