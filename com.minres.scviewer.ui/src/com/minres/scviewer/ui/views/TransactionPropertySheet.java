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
package com.minres.scviewer.ui.views;

import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.views.properties.PropertySheet;

public class TransactionPropertySheet extends PropertySheet {
	
	
	public TransactionPropertySheet() {
		super();
	}

	@Override
	protected boolean isImportant(IWorkbenchPart part) {
		return part.getSite().getId().equals("com.minres.scviewer.ui.TxEditorPart");
	}
}
