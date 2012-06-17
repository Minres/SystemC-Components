package com.itjw.txviewer.graph;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.eclipse.jface.viewers.IStructuredSelection;

import com.itjw.txviewer.graph.data.ITrStreamFacade;
import com.itjw.txviewer.graph.data.ITransactionFacade;

public class TransactionSelection implements IStructuredSelection {

	List<Object> selection = new ArrayList<Object>();
	
	public TransactionSelection(ITransactionFacade node){
		selection.add(node);
	}
	
	public TransactionSelection(List<ITransactionFacade> nodes){
		selection.addAll(nodes);
	}
	
	public TransactionSelection(ITransactionFacade currentSelection, ITrStreamFacade currentStreamSelection) {
		selection.add(currentSelection);
		if(currentStreamSelection!=null)
			selection.add(currentStreamSelection);
	}

	public void add(ITransactionFacade node){
		selection.add(node);
	}
	
	public void addAll(List<ITransactionFacade> nodes){
		selection.addAll(nodes);
	}
	
	@Override
	public boolean isEmpty() {
		return selection.size()==0;
	}

	@Override
	public ITransactionFacade getFirstElement() {
		return (ITransactionFacade)selection.get(0);
	}

	@Override
	public Iterator<Object> iterator() {
		return selection.iterator();
	}

	@Override
	public int size() {
		return selection.size();
	}

	@Override
	public Object[] toArray() {
		return selection.toArray();
	}

	@Override
	public List<Object> toList() {
		return selection;
	}

}
