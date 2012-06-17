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
