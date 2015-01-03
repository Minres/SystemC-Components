package com.minres.scviewer.database.sqlite;

import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;

import com.minres.scviewer.database.ITrHierNode;

public class HierNode implements ITrHierNode {

	protected String name;
	
	protected ArrayList<ITrHierNode> childs;
	
	public HierNode(String name) {
		this.name=name;
		childs = new ArrayList<ITrHierNode>();
	}

	@Override
	public void addPropertyChangeListener(PropertyChangeListener l) {
		// TODO Auto-generated method stub

	}

	@Override
	public void removePropertyChangeListener(PropertyChangeListener l) {
		// TODO Auto-generated method stub

	}

	@Override
	public String getFullName() {
		return name;
	}

	@Override
	public String getName() {
		return name;
	}

	@Override
	public void setName(String name) {
		this.name=name;
	}

	@Override
	public List<ITrHierNode> getChildNodes() {
		return childs;
	}

}
