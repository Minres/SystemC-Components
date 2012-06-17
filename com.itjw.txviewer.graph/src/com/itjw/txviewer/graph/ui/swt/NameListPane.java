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
package com.itjw.txviewer.graph.ui.swt;

import org.eclipse.swt.widgets.Composite;

import com.itjw.txviewer.graph.data.ITrStreamFacade;

public class NameListPane extends ListPane {

	public NameListPane(Composite parent, TxDisplay txDisplay) {
		super(parent, txDisplay);
	}

	@Override
	String getLabelValue(ITrStreamFacade str) {
		return str.getFullName();
	}

	@Override
	String getHeaderValue() {
		return "Name";
	}

}
