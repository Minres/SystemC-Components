package com.itjw.txviewer.ui;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
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
    private static final String ID_FACTORY = "com.itjw.txviewer.ui.TxEditorInputFactory"; //$NON-NLS-1$

    /**
     * Tag for the IFile.fullPath of the file resource.
     */
    private static final String TAG_PATH = "path"; //$NON-NLS-1$

    private static final String STREAMLIST_PATH = "stream_list"; //$NON-NLS-1$

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
            String listData = memento.getString(STREAMLIST_PATH);
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
		try {
	        ByteArrayOutputStream baos = new ByteArrayOutputStream();
			ObjectOutputStream oos = new ObjectOutputStream(baos);
	        oos.writeObject(input.getStreamNames());
	        memento.putString(STREAMLIST_PATH, javax.xml.bind.DatatypeConverter.printHexBinary(baos.toByteArray()));
		} catch (IOException e) {
		}
    }
}
