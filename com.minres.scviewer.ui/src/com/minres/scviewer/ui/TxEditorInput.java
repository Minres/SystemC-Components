package com.minres.scviewer.ui;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.ui.IMemento;
import org.eclipse.ui.part.FileEditorInput;

public class TxEditorInput extends FileEditorInput {

	private ArrayList<String> streamNames;
	
	public TxEditorInput(IFile file) {
		super(file);
		streamNames=new ArrayList<String>();
	}
	
    public String getFactoryId(){
    	return TxEditorInputFactory.getFactoryId();
    }

	public void saveState(IMemento memento) {
		TxEditorInputFactory.saveState(memento, this);
	}

	public List<String> getStreamNames() {
		return streamNames;
	}
	
}
