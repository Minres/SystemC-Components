package com.minres.scviewer.database.vcd;

import java.io.File;
import java.io.FileInputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Stack;
import java.util.Vector;

import com.minres.scviewer.database.BitVector;
import com.minres.scviewer.database.EventTime;
import com.minres.scviewer.database.HierNode;
import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ISignalChange;
import com.minres.scviewer.database.ISignalChangeMulti;
import com.minres.scviewer.database.ISignalChangeSingle;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDbLoader;
import com.minres.scviewer.database.InputFormatException;
import com.minres.scviewer.database.SignalChange;

// TODO: Auto-generated Javadoc
/**
 * The Class VCDDb.
 */
public class VCDDbLoader implements IWaveformDbLoader, IVCDDatabaseBuilder {

	
	private static final EventTime.Unit TIME_RES = EventTime.Unit.PS;

	private IWaveformDb db;
	
	/** The module stack. */
	private Stack<String> moduleStack;
	
	/** The signals. */
	private List<IWaveform> signals;
	
	private long maxTime;
	
	/**
	 * Instantiates a new VCD db.
	 *
	 * @param netName the net name
	 */
	public VCDDbLoader() {
	}

	private byte[] x = "$date".getBytes();

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#load(java.io.File)
	 */
	@Override
	public boolean load(IWaveformDb db, File file) throws Exception {
		this.db=db;
		FileInputStream fis = new FileInputStream(file);
		byte[] buffer = new byte[x.length];
		int read = fis.read(buffer, 0, x.length);
		fis.close();
		if (read == x.length)
			for (int i = 0; i < x.length; i++)
				if (buffer[i] != x[i])
					return false;

		signals = new Vector<IWaveform>();
		moduleStack= new Stack<String>(); 
		boolean res = new VCDFileParser(false).load(new FileInputStream(file), this);
		moduleStack=null;
		if(!res) throw new InputFormatException();
		EventTime lastTime=new EventTime(maxTime, TIME_RES);
		for(IWaveform signal:signals){
			ISignalChange change = ((ISignal<ISignalChange>)signal).getSignalChanges().last();
			if(lastTime.compareTo(change.getTime())>0){
				ISignalChange lastChange = change.duplicate();
				((SignalChange)lastChange).setTime(lastTime);
				((ISignal<ISignalChange>)signal).getSignalChanges().add(lastChange);
			}
		}
		return true;
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#getMaxTime()
	 */
	@Override
	public EventTime getMaxTime() {
		return new EventTime(maxTime, TIME_RES);
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#getAllWaves()
	 */
	@Override
	public List<IWaveform> getAllWaves() {
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
	@Override
	public Integer newNet(String netName, int i, int width) {
		int id = signals.size();
		VCDSignal<? extends ISignalChange> signal;
		if(width==1){
			signal = i<0 ? new VCDSignal<ISignalChangeSingle>(db, id, netName) :
				new VCDSignal<ISignalChangeSingle>(signals.get(i), id, netName);
		} else {
			signal = i<0 ? new VCDSignal<ISignalChangeMulti>(db, id, netName, width) :
				new VCDSignal<ISignalChangeMulti>(signals.get(i), id, netName);
		};
		signals.add(signal);
		return id;
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.vcd.ITraceBuilder#getNetWidth(int)
	 */
	@SuppressWarnings("unchecked")
	@Override
	public int getNetWidth(int intValue) {
		VCDSignal<? extends ISignalChange> signal = (VCDSignal<? extends ISignalChange>) signals.get(intValue);
		return signal.getWidth();
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.vcd.ITraceBuilder#appendTransition(int, long, com.minres.scviewer.database.vcd.BitVector)
	 */
	@SuppressWarnings("unchecked")
	@Override
	public void appendTransition(int intValue, long fCurrentTime, BitVector decodedValues) {
		VCDSignal<? extends ISignalChange> signal = (VCDSignal<? extends ISignalChange>) signals.get(intValue);
		EventTime time = new EventTime(fCurrentTime, TIME_RES);
		if(signal.getWidth()==1){
			VCDSignalChangeSingle change = new VCDSignalChangeSingle(time, decodedValues.getValue()[0]);
			((VCDSignal<ISignalChangeSingle>)signal).addSignalChange(change);
		} else {
			VCDSignalChangeMulti change = new VCDSignalChangeMulti(time, decodedValues);
			((VCDSignal<VCDSignalChangeMulti>)signal).addSignalChange(change);			
		}
		maxTime= Math.max(maxTime, fCurrentTime);
	}

}
