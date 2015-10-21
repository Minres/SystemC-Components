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
package com.minres.scviewer.database.sqlite;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.IWaveformEvent;

public class TxEvent implements ITxEvent {

	private final Type type;
	private ITx tx;
	
	public TxEvent(Type type, ITx tx) {
		super();
		this.type = type;
		this.tx = tx;
	}

	@Override
	public Long getTime() {
		return type==Type.BEGIN?tx.getBeginTime():tx.getEndTime();
	}

	@Override
	public IWaveformEvent duplicate() throws CloneNotSupportedException {
		return new TxEvent(type, tx);
	}

	@Override
	public int compareTo(IWaveformEvent o) {
		return getTime().compareTo(o.getTime());
	}

	@Override
	public ITx getTransaction() {
		return tx;
	}

	@Override
	public Type getType() {
		return type;
	}

	@Override
	public String toString() {
		return type.toString()+"@"+getTime()+" of tx #"+tx.getId();
	}
}
