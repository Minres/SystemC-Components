package com.minres.scviewer.database.vcd;

import com.minres.scviewer.database.BitVector;
import com.minres.scviewer.database.EventTime;
import com.minres.scviewer.database.ISignalChangeMulti;
import com.minres.scviewer.database.SignalChange;

public class VCDSignalChangeMulti extends SignalChange implements ISignalChangeMulti, Cloneable  {

	private BitVector value;
	
	public VCDSignalChangeMulti(EventTime time) {
		super(time);
	}

	public VCDSignalChangeMulti(EventTime time, BitVector decodedValues) {
		super(time);
		this.value=decodedValues;
	}

	public BitVector getValue() {
		return value;
	}
	
	public void setValue(BitVector value) {
		this.value = value;
	}

}
