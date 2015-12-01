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

import java.io.File;
import java.io.FileInputStream;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Stack;
import java.util.TreeMap;
import java.util.Vector;

import com.minres.scviewer.database.BitVector;
import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ISignalChange;
import com.minres.scviewer.database.ISignalChangeMulti;
import com.minres.scviewer.database.ISignalChangeSingle;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformDbLoader;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.InputFormatException;
import com.minres.scviewer.database.RelationType;

/**
 * The Class VCDDb.
 */
public class VCDDbLoader implements IWaveformDbLoader, IVCDDatabaseBuilder {

	
	/** The Constant TIME_RES. */
	private static final Long TIME_RES = 1000L; // ps;

	/** The db. */
	private IWaveformDb db;
	
	/** The module stack. */
	private Stack<String> moduleStack;
	
	/** The signals. */
	private List<IWaveform<? extends IWaveformEvent>> signals;
	
	/** The max time. */
	private long maxTime;
	
	/**
	 * Instantiates a new VCD db.
	 */
	public VCDDbLoader() {
	}

	/** The date bytes. */
	private byte[] dateBytes = "$date".getBytes();

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#load(java.io.File)
	 */
	@SuppressWarnings("unchecked")
	@Override
	public boolean load(IWaveformDb db, File file) throws Exception {
		this.db=db;
		FileInputStream fis = new FileInputStream(file);
		byte[] buffer = new byte[dateBytes.length];
		int read = fis.read(buffer, 0, dateBytes.length);
		fis.close();
		if (read == dateBytes.length)
			for (int i = 0; i < dateBytes.length; i++)
				if (buffer[i] != dateBytes[i])
					return false;

		signals = new Vector<IWaveform<? extends IWaveformEvent>>();
		moduleStack= new Stack<String>(); 
		boolean res = new VCDFileParser(false).load(new FileInputStream(file), this);
		moduleStack=null;
		if(!res) throw new InputFormatException();
		// calculate max time of database
		for(IWaveform<? extends IWaveformEvent> waveform:signals)
			maxTime= Math.max(maxTime, ((ISignal<? extends ISignalChange>)waveform).getEvents().lastKey());
		// extend signals to hav a last value set at max time
		for(IWaveform<? extends IWaveformEvent> waveform:signals){
			TreeMap<Long,? extends ISignalChange> events = ((VCDSignal<? extends ISignalChange>)waveform).values;
			if(events.lastKey()<maxTime){
				ISignalChange x = events.lastEntry().getValue();
				if(x instanceof ISignalChangeSingle)
					((VCDSignal<ISignalChangeSingle>)waveform).values.put(maxTime, 
							new VCDSignalChangeSingle(maxTime, ((ISignalChangeSingle)x).getValue()));
				else
					if(x instanceof ISignalChangeMulti)
						((VCDSignal<ISignalChangeMulti>)waveform).values.put(maxTime, 
								new VCDSignalChangeMulti(maxTime, ((ISignalChangeMulti)x).getValue()));
			}
		}
		return true;
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#getMaxTime()
	 */
	@Override
	public Long getMaxTime() {
		return maxTime;
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#getAllWaves()
	 */
	@Override
	public List<IWaveform<? extends IWaveformEvent>> getAllWaves() {
		return signals;
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.vcd.ITraceBuilder#enterModule(java.lang.String)
	 */
	@Override
	public void enterModule(String tokenString) {
		if(moduleStack.isEmpty())
			moduleStack.push(tokenString);
		else
			moduleStack.push(moduleStack.peek()+"."+tokenString);

	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.vcd.ITraceBuilder#exitModule()
	 */
	@Override
	public void exitModule() {
		if(!moduleStack.isEmpty()) moduleStack.pop();
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.vcd.ITraceBuilder#newNet(java.lang.String, int, int)
	 */
	@SuppressWarnings("unchecked")
	@Override
	public Integer newNet(String netName, int i, int width) {
		int id = signals.size();
		VCDSignal<? extends IWaveformEvent> signal;
		if(width==1){
			signal = i<0 ? new VCDSignal<ISignalChangeSingle>(db, id, netName) :
				new VCDSignal<ISignalChangeSingle>((VCDSignal<ISignalChangeSingle>)signals.get(i), id, netName);
		} else {
			signal = i<0 ? new VCDSignal<ISignalChangeMulti>(db, id, netName, width) :
				new VCDSignal<ISignalChangeMulti>((VCDSignal<VCDSignalChangeMulti>)signals.get(i), id, netName);
		};
		signals.add(signal);
		return id;
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.vcd.ITraceBuilder#getNetWidth(int)
	 */
	@Override
	public int getNetWidth(int intValue) {
		VCDSignal<? extends IWaveformEvent> signal = (VCDSignal<? extends IWaveformEvent>) signals.get(intValue);
		return signal.getWidth();
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.vcd.ITraceBuilder#appendTransition(int, long, com.minres.scviewer.database.vcd.BitVector)
	 */
	@SuppressWarnings("unchecked")
	@Override
	public void appendTransition(int signalId, long currentTime, BitVector decodedValues) {
		VCDSignal<? extends IWaveformEvent> signal = (VCDSignal<? extends IWaveformEvent>) signals.get(signalId);
		Long time = currentTime* TIME_RES;
		if(signal.getWidth()==1){
			((VCDSignal<ISignalChangeSingle>)signal).values.put(time, new VCDSignalChangeSingle(time, decodedValues.getValue()[0]));
		} else {
			((VCDSignal<VCDSignalChangeMulti>)signal).values.put(time, new VCDSignalChangeMulti(time, decodedValues));
		}
	}
	
	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.IWaveformDbLoader#getAllRelationTypes()
	 */
	@Override
	public Collection<RelationType> getAllRelationTypes(){
		return Collections.emptyList();
	}

}
