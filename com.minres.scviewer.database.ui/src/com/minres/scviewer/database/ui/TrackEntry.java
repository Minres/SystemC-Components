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
package com.minres.scviewer.database.ui;

import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ISignalChange;
import com.minres.scviewer.database.ITxEvent;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformEvent;

public class TrackEntry {
	final public IWaveform<? extends IWaveformEvent> waveform;

	public int vOffset;
	
	public int height;

	public boolean selected;
	
	public TrackEntry(IWaveform<? extends IWaveformEvent> waveform) {
		this.waveform = waveform;
		vOffset=0;
		height=0;
		selected=false;
	}
	
	public boolean isStream(){
		return waveform instanceof ITxStream<?>;
	}

	public ITxStream<? extends ITxEvent> getStream(){
		return (ITxStream<?>) waveform;
	}

	public boolean isSignal(){
		return waveform instanceof ISignal<?>;
	}
	
	public ISignal<? extends ISignalChange> getSignal(){
		return (ISignal<?>) waveform;
	}

	@Override
    public boolean equals(Object obj) {
		if(obj instanceof TrackEntry){
			TrackEntry o = (TrackEntry) obj;
			return waveform==o.waveform && vOffset==o.vOffset;
		}
		return false;
	}
}