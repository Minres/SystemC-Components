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

import com.minres.scviewer.database.ISignalChangeSingle;
import com.minres.scviewer.database.SignalChange;

public class VCDSignalChangeSingle extends SignalChange implements ISignalChangeSingle, Cloneable {

	private char value;

	public VCDSignalChangeSingle(Long time, char value) {
		super(time);
		this.value=value;
	}

	public char getValue() {
		return value;
	}

	public void setValue(char value) {
		this.value = value;
	}
	
	@Override
	public String toString() {
		return value+"@"+getTime();
	}
}
