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
package com.minres.scviewer.ui;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.ui.IMemento;
import org.eclipse.ui.part.FileEditorInput;

public class TxEditorInput extends FileEditorInput {

	private ArrayList<String> streamNames;
	
	private Boolean secondaryLoaded=null;
	
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

	public Boolean isSecondaryLoaded() {
		return secondaryLoaded;
	}

	public void setSecondaryLoaded(Boolean secondaryLoaded) {
		this.secondaryLoaded = secondaryLoaded;
	}
	
}
