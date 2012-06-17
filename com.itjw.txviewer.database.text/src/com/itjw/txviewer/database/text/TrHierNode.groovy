package com.itjw.txviewer.database.text;

import java.beans.PropertyChangeListener;
import java.util.List;
import java.util.Map;

import com.itjw.txviewer.database.ITrHierNode;

class TrHierNode implements ITrHierNode {

	String name
	def childs = []

	public TrHierNode(){
	}

	public TrHierNode(String name){
		this.name=name
	}
	
	@Override
	public List<ITrHierNode>  getChildNodes() {
		return childs.sort{it.name}
	}

	@Override
	public String getFullName() {
		// TODO Auto-generated method stub
		return name;
	}

	@Override
	public void addPropertyChangeListener(PropertyChangeListener l) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void removePropertyChangeListener(PropertyChangeListener l) {
		// TODO Auto-generated method stub
		
	}

}
