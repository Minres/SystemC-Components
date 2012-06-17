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
