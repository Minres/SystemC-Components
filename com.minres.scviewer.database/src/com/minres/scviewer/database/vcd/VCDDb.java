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
import com.minres.scviewer.database.InputFormatException;
import com.minres.scviewer.database.SignalChange;

// TODO: Auto-generated Javadoc
/**
 * The Class VCDDb.
 */
public class VCDDb extends HierNode implements IWaveformDb, IVCDDatabaseBuilder {

	
	private static final String TIME_RES = "ps";

	/** The module stack. */
	private Stack<String> moduleStack;
	
	/** The signals. */
	private List<IWaveform> signals;
	
	private HashMap<String, IWaveform> waveformLookup;
	
	private long maxTime;
	
	/**
	 * Instantiates a new VCD db.
	 *
	 * @param netName the net name
	 */
	public VCDDb() {
		super("VCDDb");
		signals = new Vector<IWaveform>();
		waveformLookup = new HashMap<String, IWaveform>();
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#getStreamByName(java.lang.String)
	 */
	@Override
	public IWaveform getStreamByName(String name) {
		return waveformLookup.get(name);
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#load(java.io.File)
	 */
	@SuppressWarnings("unchecked")
	@Override
	public void load(File inp) throws Exception {
		moduleStack= new Stack<String>(); 
		boolean res = new VCDFileParser(false).load(new FileInputStream(inp), this);
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
		buildHierarchyNodes();
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.ITrDb#clear()
	 */
	@Override
	public void clear() {
		signals.clear();
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
			signal = i<0 ? new VCDSignal<ISignalChangeSingle>(id, netName) :
				new VCDSignal<ISignalChangeSingle>(signals.get(i), id, netName);
		} else {
			signal = i<0 ? new VCDSignal<ISignalChangeMulti>(id, netName, width) :
				new VCDSignal<ISignalChangeMulti>(signals.get(i), id, netName);
		};
		signals.add(signal);
		return id;
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.vcd.ITraceBuilder#getNetWidth(int)
	 */
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

	private void buildHierarchyNodes() throws InputFormatException{
		for(IWaveform stream:getAllWaves()){
			waveformLookup.put(stream.getFullName(), stream);
			String[] hier = stream.getFullName().split("\\.");
			IHierNode node = this;
			for(String name:hier){
				IHierNode n1 = null;
				 for (IHierNode n : node.getChildNodes()) {
				        if (n.getName().equals(name)) {
				            n1=n;
				            break;
				        }
				    }
				if(name == hier[hier.length-1]){ //leaf
					if(n1!=null) {
						if(n1 instanceof HierNode){
							node.getChildNodes().remove(n1);
							stream.getChildNodes().addAll(n1.getChildNodes());
						} else {
							throw new InputFormatException();
						}
					}
					stream.setName(name);
					node.getChildNodes().add(stream);
					node=stream;
				} else { // intermediate
					if(n1 != null) {
						node=n1;
					} else {
						HierNode newNode = new HierNode(name);
						node.getChildNodes().add(newNode);
						node=newNode;
					}
				}
			}
		}
	}


}
