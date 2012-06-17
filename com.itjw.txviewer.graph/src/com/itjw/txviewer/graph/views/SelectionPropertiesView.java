package com.itjw.txviewer.graph.views;

import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.views.properties.PropertySheet;

public class SelectionPropertiesView extends PropertySheet {
	@Override
	protected boolean isImportant(IWorkbenchPart part) {
		return part.getSite().getId().equals("com.itjw.txviewer.graph.TxEditorPart");
	}
}
