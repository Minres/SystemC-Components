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

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;

import com.itjw.txviewer.graph.data.ITrStreamFacade;

public class ValueListPane extends ListPane {

	public ValueListPane(Composite parent, TxDisplay txDisplay) {
		super(parent, txDisplay);
	}

	@Override
	String getLabelValue(ITrStreamFacade str) {
		return str.getValueAtCursor(0);
	}

	@Override
	String getHeaderValue() {
		return "Value";
	}

	protected void formatLabel(Label l, ITrStreamFacade str, int trackIdx){
		super.formatLabel(l, str, trackIdx);
		l.setAlignment(SWT.CENTER);
	}

}
