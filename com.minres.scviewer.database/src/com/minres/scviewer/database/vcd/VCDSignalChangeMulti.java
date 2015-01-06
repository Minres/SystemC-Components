package com.minres.scviewer.database.vcd;

import com.minres.scviewer.database.EventTime;
import com.minres.scviewer.database.ISignalChangeMulti;
import com.minres.scviewer.database.SignalChange;

public class VCDSignalChangeMulti extends SignalChange implements ISignalChangeMulti, Cloneable  {

	private String value;
	
	public VCDSignalChangeMulti(EventTime time) {
		super(time);
	}

	public VCDSignalChangeMulti(EventTime time, String value) {
		super(time);
		this.value=value;
	}

	public String getValue() {
		return value;
	}
	
	public void setValue(String value) {
		this.value = value;
	}

}
