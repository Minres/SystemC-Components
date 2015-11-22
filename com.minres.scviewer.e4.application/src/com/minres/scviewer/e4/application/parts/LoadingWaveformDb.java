package com.minres.scviewer.e4.application.parts;

import java.beans.PropertyChangeListener;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformEvent;
import com.minres.scviewer.database.RelationType;

public class LoadingWaveformDb implements IWaveformDb {

	private final String label = "Database loading..."; 

	@Override
	public void addPropertyChangeListener(PropertyChangeListener l) {
	}

	@Override
	public void removePropertyChangeListener(PropertyChangeListener l) {
	}

	@Override
	public String getFullName() {
		return label;
	}

	@Override
	public String getName() {
		return label;
	}

	@Override
	public void setName(String name) {
	}

	@Override
	public void setParentName(String name) {
	}

	@Override
	public List<IHierNode> getChildNodes() {
		return new ArrayList<IHierNode>();
	}

	@Override
	public int compareTo(IHierNode o) {
		return 0;
	}

	@Override
	public Long getMaxTime() {
		return new Long(0);
	}

	@Override
	public IWaveform<? extends IWaveformEvent> getStreamByName(String name) {
		return null;
	}

	@Override
	public List<IWaveform<?>> getAllWaves() {
		return new ArrayList<IWaveform<?>>();
	}

	@Override
	public List<RelationType> getAllRelationTypes() {
		return new ArrayList<RelationType>();
	}

	@Override
	public boolean load(File inp) throws Exception {
		return false;
	}

	@Override
	public boolean isLoaded() {
		return false;
	}

	@Override
	public void clear() {
	}

}
