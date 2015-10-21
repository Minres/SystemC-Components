/*******************************************************************************
 * Copyright (c) 2014, 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.e4.application.provider;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.viewers.ILabelProvider;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.wb.swt.ResourceManager;

import com.minres.scviewer.database.IHierNode;
import com.minres.scviewer.database.ISignal;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveformDb;

public class TxDbLabelProvider implements ILabelProvider {

	private List<ILabelProviderListener> listeners = new ArrayList<ILabelProviderListener>();

	private Image database;
	private Image stream;
	private Image signal;
	private Image folder;
	
	
	public TxDbLabelProvider() {
		super();
		database=ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/database.png");
		stream=ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/stream.png");
		folder=ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/folder.png");
		signal=ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/signal.png");
	}

	@Override
	public void addListener(ILabelProviderListener listener) {
		  listeners.add(listener);
	}

	@Override
	public void dispose() {
		if(database!=null) database.dispose();
		if(stream!=null) stream.dispose();
		if(folder!=null) folder.dispose();
		if(signal!=null) signal.dispose();
	}

	@Override
	public boolean isLabelProperty(Object element, String property) {
		  return false;
	}

	@Override
	public void removeListener(ILabelProviderListener listener) {
		  listeners.remove(listener);
	}

	@Override
	public Image getImage(Object element) {
		if(element instanceof IWaveformDb){
			return database;
		}else if(element instanceof ITxStream){
			return stream;
		}else if(element instanceof ISignal<?>){
			return signal;
		}else if(element instanceof IHierNode){
			return folder;
		} else
			return null;
	}

	@Override
	public String getText(Object element) {
		return ((IHierNode)element).getName();
	}

}


