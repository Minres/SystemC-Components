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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.Path;
import org.eclipse.ui.IElementFactory;
import org.eclipse.ui.IMemento;

public class TxEditorInputFactory implements IElementFactory {

	   /**
     * Factory id. The workbench plug-in registers a factory by this name
     * with the "org.eclipse.ui.elementFactories" extension point.
     */
    private static final String ID_FACTORY = "com.minres.scviewer.ui.TxEditorInputFactory"; //$NON-NLS-1$
    /**
     * Tag for the IFile.fullPath of the file resource.
     */
    private static final String TAG_PATH = "path"; //$NON-NLS-1$
    /**
     * Tag for the secondaryLoade of the resource.
     */
    private static final String TAG_SECONDARY = "secondary"; //$NON-NLS-1$
    /**
     * Tag for the streamList of the resource.
     */
    private static final String TAG_STREAMLIST = "stream_list"; //$NON-NLS-1$

    /**
     * Creates a new factory.
     */
    public TxEditorInputFactory() {
    }

    /* (non-Javadoc)
     * Method declared on IElementFactory.
     */
    @SuppressWarnings("unchecked")
	public IAdaptable createElement(IMemento memento) {
        // Get the file name.
        String fileName = memento.getString(TAG_PATH);
        if (fileName == null) {
			return null;
		}
        // Get a handle to the IFile...which can be a handle
        // to a resource that does not exist in workspace
        IFile file = ResourcesPlugin.getWorkspace().getRoot().getFile(new Path(fileName));
        if (file != null) {
        	TxEditorInput tei = new TxEditorInput(file);
            Boolean isSecondaryLoaded = memento.getBoolean(TAG_SECONDARY);
            if(isSecondaryLoaded!=null)
            	tei.setSecondaryLoaded(isSecondaryLoaded);
            String listData = memento.getString(TAG_STREAMLIST);
            if (listData != null) {
				try {
	    	        ByteArrayInputStream bais = new ByteArrayInputStream(javax.xml.bind.DatatypeConverter.parseHexBinary(listData));
	    	        ObjectInputStream ois = new ObjectInputStream(bais);
	    	        Object obj = ois.readObject();
	    	        if(obj instanceof List<?>)
	    	        	tei.getStreamNames().addAll((List<String>)obj);
				} catch (Exception e) {
				}
            }
        	return tei;
		}
		return null;
    }

    /**
     * Returns the element factory id for this class.
     * 
     * @return the element factory id
     */
    public static String getFactoryId() {
        return ID_FACTORY;
    }

    /**
     * Saves the state of the given file editor input into the given memento.
     *
     * @param memento the storage area for element state
     * @param input the file editor input
     */
    public static void saveState(IMemento memento, TxEditorInput input) {
        IFile file = input.getFile();
        memento.putString(TAG_PATH, file.getFullPath().toString());
        if(input.isSecondaryLoaded()!=null) memento.putBoolean(TAG_SECONDARY, input.isSecondaryLoaded());
		try {
	        ByteArrayOutputStream baos = new ByteArrayOutputStream();
			ObjectOutputStream oos = new ObjectOutputStream(baos);
	        oos.writeObject(input.getStreamNames());
	        memento.putString(TAG_STREAMLIST, javax.xml.bind.DatatypeConverter.printHexBinary(baos.toByteArray()));
		} catch (IOException e) {
		}
    }
}
