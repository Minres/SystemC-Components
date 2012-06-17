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
package com.itjw.txviewer.graph.views;

import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.views.properties.PropertySheet;

public class SelectionPropertiesView extends PropertySheet {
	@Override
	protected boolean isImportant(IWorkbenchPart part) {
		return part.getSite().getId().equals("com.itjw.txviewer.graph.TxEditorPart");
	}
}
