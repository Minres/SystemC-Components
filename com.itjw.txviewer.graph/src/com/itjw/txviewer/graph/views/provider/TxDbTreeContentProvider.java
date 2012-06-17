package com.itjw.txviewer.graph.views.provider;

import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;

import com.itjw.txviewer.database.ITrDb;
import com.itjw.txviewer.database.ITrHierNode;

public class TxDbTreeContentProvider implements ITreeContentProvider {

	private ITrDb database;
	
	@Override
	public void dispose() {	}

	@Override
	public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
		database=(ITrDb)newInput;
	}

	@Override
	public Object[] getElements(Object inputElement) {
		return database.getChildNodes().toArray();
	}

	@Override
	public Object[] getChildren(Object parentElement) {
		if(parentElement instanceof ITrHierNode){
			return ((ITrHierNode)parentElement).getChildNodes().toArray();
		}
		return null;
	}

	@Override
	public Object getParent(Object element) {
		return null;
	}

	@Override
	public boolean hasChildren(Object element) {
		Object[] obj = getChildren(element);
	    return obj == null ? false : obj.length > 0;
	}

}
