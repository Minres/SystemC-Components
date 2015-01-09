/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.ui;

import java.io.File;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorSite;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.FileStoreEditorInput;
import org.eclipse.ui.part.EditorPart;
import org.eclipse.ui.views.contentoutline.IContentOutlinePage;
import org.eclipse.ui.views.properties.IPropertySheetPage;
import org.eclipse.ui.views.properties.tabbed.ITabbedPropertySheetPageContributor;
import org.eclipse.ui.views.properties.tabbed.TabbedPropertySheetPage;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.ServiceReference;

import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.IWaveform;
import com.minres.scviewer.database.WaveformDb;
import com.minres.scviewer.ui.handler.GotoDirection;
import com.minres.scviewer.ui.swt.TxDisplay;
import com.minres.scviewer.ui.views.TxOutlinePage;

public class TxEditorPart extends EditorPart implements ITabbedPropertySheetPageContributor {

	public static final String ID = "com.minres.scviewer.ui.TxEditorPart"; //$NON-NLS-1$

	public static final String WAVE_ACTION_ID = "com.minres.scviewer.ui.action.AddToWave";

	private TxDisplay txDisplay;
	
	/** This is the root of the editor's model. */
	private IWaveformDb database;

	private Composite myParent;

	public TxEditorPart() {
	}

	/**
	 * Create contents of the editor part.
	 * @param parent
	 */
	@Override
	public void createPartControl(Composite parent) {
		myParent=parent;		
		txDisplay = new TxDisplay(parent);
		if(database!=null) database.addPropertyChangeListener(txDisplay); 
		getSite().setSelectionProvider(txDisplay);
		if(getEditorInput()!=null && ((TxEditorInput) getEditorInput()).getStreamNames().size()>0 && database!=null){
			if(MessageDialog.openConfirm(parent.getShell(), "Confirm", "Do you want the restore last state of the wave form?"))
				for(String streamName:((TxEditorInput) getEditorInput()).getStreamNames()){
					IWaveform stream = database.getStreamByName(streamName);
					if(stream!=null)
						txDisplay.addStream(stream);
				}
		}
	}

	
	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.EditorPart#setInput(org.eclipse.ui.IEditorInput)
	 */
	protected void setInput(IEditorInput input) {
		super.setInput(input);
		try {
			if(input instanceof IFileEditorInput){
				if(!(input instanceof TxEditorInput))
					super.setInput(new TxEditorInput(((IFileEditorInput)input).getFile()));
				IPath location = ((IFileEditorInput) input).getFile().getLocation();
				if (location != null) loadDatabases(location.toFile());
			} else if(input instanceof FileStoreEditorInput){
				File file=new File(((FileStoreEditorInput) input).getURI().getPath());
				loadDatabases(file);
			}
		} catch (Exception e) {
			handleLoadException(e);
		}
		if(database!=null) setPartName(database.getName());
	}

	protected void loadDatabases(File file) throws Exception {
		database=new WaveformDb();
		if(txDisplay !=null) database.addPropertyChangeListener(txDisplay);
		if(database.load(file)){
			String ext = getFileExtension(file.getName());
			if("vcd".equals(ext.toLowerCase())){
				File txFile = new File(renameFileExtension(file.getCanonicalPath(), "txdb"));
				if(txFile.exists() && database.load(txFile)) return;
				txFile = new File(renameFileExtension(file.getCanonicalPath(), "txlog"));
				if(txFile.exists())	database.load(txFile);
			} else if("txdb".equals(ext.toLowerCase()) || "txlog".equals(ext.toLowerCase())){
				File txFile = new File(renameFileExtension(file.getCanonicalPath(), "vcd"));
				if(txFile.exists()) database.load(txFile);				
			}
		} else {
			MessageDialog.openError(PlatformUI.getWorkbench().getDisplay().getActiveShell(),
					"Error loading database", "Could not find an usable and applicable database loader implementation");
			database=null;
		}
	}
	
	protected static String renameFileExtension(String source, String newExt) {
	    String target;
	    String currentExt = getFileExtension(source);
	    if (currentExt.equals("")){
	      target=source+"."+newExt;
	    } else {
	      target=source.replaceFirst(Pattern.quote("."+currentExt)+"$", Matcher.quoteReplacement("."+newExt));
	    }
	    return target;
	  }

	protected static String getFileExtension(String f) {
	    String ext = "";
	    int i = f.lastIndexOf('.');
	    if (i > 0 &&  i < f.length() - 1) {
	      ext = f.substring(i + 1);
	    }
	    return ext;
	  }
	  
	private void handleLoadException(Exception e) {
		MessageDialog.openError(PlatformUI.getWorkbench().getDisplay().getActiveShell(),
				"Error loading database", e.getMessage());
		database = null;
	}

	@Override
	public void setFocus() {
		myParent.setFocus();
	}

	@Override
	public void doSave(IProgressMonitor monitor) {
	}

	@Override
	public void doSaveAs() {
	}

	@SuppressWarnings("rawtypes")
	@Override
	public Object getAdapter(Class type) {
		if (type == IContentOutlinePage.class) // outline page
			return new TxOutlinePage(this);
		else if (type == IPropertySheetPage.class) // use tabbed property sheet instead of standard one
            return new TabbedPropertySheetPage(this);
		return super.getAdapter(type);
	}

	public IWaveformDb getModel() {
		return database;
	}

	@Override
	public void init(IEditorSite site, IEditorInput input)
			throws PartInitException {
		// Initialize the editor part
		setSite(site);
		setInput(input);
	}

	@Override
	public boolean isDirty() {
		return false;
	}

	@Override
	public boolean isSaveAsAllowed() {
		return false;
	}

	public IWaveformDb getDatabase() {
		return database;
	}

	public void addStreamToList(IWaveform obj){
		if(getEditorInput() instanceof TxEditorInput && !((TxEditorInput) getEditorInput()).getStreamNames().contains(obj.getFullName())){
			((TxEditorInput) getEditorInput()).getStreamNames().add(obj.getFullName());
			txDisplay.addStream(obj);
		} else
			txDisplay.addStream(obj);
			
	}
	
	public void addStreamsToList(IWaveform[] iWaveforms){
		for(IWaveform stream:iWaveforms)
			addStreamToList(stream);
	}

	public void removeStreamFromList(IWaveform obj){
		if(getEditorInput() instanceof TxEditorInput && ((TxEditorInput) getEditorInput()).getStreamNames().contains(obj.getFullName())){
			((TxEditorInput) getEditorInput()).getStreamNames().remove(obj.getFullName());
			txDisplay.removeStream(obj);
		} else
			txDisplay.removeStream(obj);
	}
	
	public void removeStreamsFromList(IWaveform[] iWaveforms){
		for(IWaveform stream:iWaveforms)
			removeStreamFromList(stream);
	}
	
	public List<IWaveform> getStreamList(){
		return txDisplay.getStreamList();
	}

	public void setSelection(final ISelection selection){
		myParent.getDisplay().asyncExec(new Runnable() {
			@Override
			public void run() {
				txDisplay.setSelection(selection);
			}
		});
	}
	
	@Override
	public String getContributorId() {
		return getSite().getId();
	}

	public void moveSelection(GotoDirection next) {
		txDisplay.moveSelection( next);		
	}

}
