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
package com.minres.scviewer.database;

public class SignalChange implements IWaveformEvent {
	
	Long time;
	
	
	public SignalChange() {
		time=0L;
	}

	public SignalChange(Long time) {
		super();
		this.time = time;
	}

	@Override
	public int compareTo(IWaveformEvent o) {
		return time.compareTo(o.getTime());
	}

	@Override
	public Long getTime() {
		return time;
	}

	public void setTime(Long time) {
		this.time = time;
	}

	@Override
	public IWaveformEvent duplicate() throws CloneNotSupportedException {
		return (IWaveformEvent) this.clone();
	}

}
