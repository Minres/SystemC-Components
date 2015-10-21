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
package com.minres.scviewer.database.vcd;

import com.minres.scviewer.database.BitVector;
import com.minres.scviewer.database.ISignalChangeMulti;
import com.minres.scviewer.database.SignalChange;

public class VCDSignalChangeMulti extends SignalChange implements ISignalChangeMulti, Cloneable  {

	private BitVector value;
	
	public VCDSignalChangeMulti(Long time) {
		super(time);
	}

	public VCDSignalChangeMulti(Long time, BitVector decodedValues) {
		super(time);
		this.value=decodedValues;
	}

	public BitVector getValue() {
		return value;
	}
	
	public void setValue(BitVector value) {
		this.value = value;
	}

	@Override
	public String toString() {
		return value.toHexString()+"@"+getTime();
	}

}
