package com.minres.scviewer.database.vcd;

import com.minres.scviewer.database.EventTime;
import com.minres.scviewer.database.ISignalChangeSingle;
import com.minres.scviewer.database.SignalChange;

public class VCDSignalChangeSingle extends SignalChange implements ISignalChangeSingle, Cloneable {

	private char value;

	public VCDSignalChangeSingle(EventTime time, char value) {
		super(time);
		this.value=value;
	}

	public char getValue() {
		return value;
	}

	public void setValue(char value) {
		this.value = value;
	}
	
}
