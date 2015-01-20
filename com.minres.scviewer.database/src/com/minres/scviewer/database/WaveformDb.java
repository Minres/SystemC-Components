package com.minres.scviewer.database;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class WaveformDb extends HierNode implements IWaveformDb {

	private static List<IWaveformDbLoader> loaders=new LinkedList<IWaveformDbLoader>();

	private List<IHierNode> childNodes;

	private Map<String, IWaveform<?>> waveforms;

	private Long maxTime;
	
	
	public void bind(IWaveformDbLoader loader){
		loaders.add(loader);
	}

	public void unbind(IWaveformDbLoader loader){
		loaders.remove(loader);
	}

	
	public static List<IWaveformDbLoader> getLoaders() {
		return Collections.unmodifiableList(loaders);
	}

	public WaveformDb() {
		super();
		waveforms = new HashMap<String, IWaveform<?>>();
		maxTime=0L;
	}

	@Override
	public Long getMaxTime() {
		return maxTime;
	}

	@Override
	public IWaveform<?> getStreamByName(String name) {
		return waveforms.get(name);
	}

	@Override
	public List<IWaveform<?>> getAllWaves() {
		return new ArrayList<IWaveform<?>>(waveforms.values());
	}

	@Override
	public boolean load(File inp) throws Exception {
		for(IWaveformDbLoader loader:loaders){
			if(loader.load(this, inp)){
				for(IWaveform<?> w:loader.getAllWaves()){
					waveforms.put(w.getFullName(),w);
				}
				if(loader.getMaxTime()>maxTime){
					maxTime=loader.getMaxTime();
				}
				buildHierarchyNodes() ;
				if(name==null) name=getFileBasename(inp.getName());
				pcs.firePropertyChange("WAVEFORMS", null, waveforms);
				pcs.firePropertyChange("CHILDS", null, childNodes);
				return true;
			}
		}		
		return false;
	}

	protected static String getFileBasename(String f) {
	    String ext = "";
	    int i = f.lastIndexOf('.');
	    if (i > 0 &&  i < f.length() - 1) {
	      ext = f.substring(0, i);
	    }
	    return ext;
	  }

	@Override
	public void clear() {
		waveforms.clear();
		childNodes.clear();
	}

	private void buildHierarchyNodes() throws InputFormatException{
		childNodes= new ArrayList<IHierNode>();
		for(IWaveform<?> stream:getAllWaves()){
			updateMaxTime(stream);
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
							Collections.sort(stream.getChildNodes());
						} else {
							throw new InputFormatException();
						}
					}
					stream.setName(name);
					node.getChildNodes().add(stream);
					Collections.sort(node.getChildNodes());
					node=stream;
				} else { // intermediate
					if(n1 != null) {
						node=n1;
					} else {
						HierNode newNode = new HierNode(name);
						node.getChildNodes().add(newNode);
						Collections.sort(node.getChildNodes());
						node=newNode;
					}
				}
			}
		}
	}

	private void updateMaxTime(IWaveform<?> waveform) {
		Long last=0L;
		if(waveform instanceof ITxStream<?>)
			last=((ITxStream<?>)waveform).getEvents().lastEntry().getKey();
		else if(waveform instanceof ISignal<?>)
			last=((ISignal<?>)waveform).getEvents().lastEntry().getKey();
		if(last>maxTime)
			maxTime=last;
	}

}
