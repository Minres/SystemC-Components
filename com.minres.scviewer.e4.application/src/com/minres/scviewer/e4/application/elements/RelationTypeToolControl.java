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
package com.minres.scviewer.e4.application.elements;

import javax.annotation.PostConstruct;
import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.ui.model.application.ui.basic.MPart;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.EPartService;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.ComboViewer;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.RelationType;
import com.minres.scviewer.e4.application.parts.PartListener;
import com.minres.scviewer.e4.application.parts.WaveformViewer;

/**
 * The Class RelationTypeToolControl allowing to control which TX relation is used for navigation.
 */
public class RelationTypeToolControl extends PartListener implements ISelectionChangedListener {
	
	/** The part service. */
	EPartService partService;
	
	/** The combo viewer. */
	ComboViewer comboViewer;
	
	/** The waveform viewer part. */
	WaveformViewer waveformViewerPart;
	
	/** The dummy. */
	RelationType dummy = RelationType.create("------------");
	
	/**
	 * Instantiates a new relation type tool control.
	 *
	 * @param partService the part service
	 */
	@Inject
	public RelationTypeToolControl(EPartService partService) {
		this.partService=partService;
		partService.addPartListener(this);
	}
	
	/**
	 * Creates the gui.
	 *
	 * @param parent the parent
	 */
	@PostConstruct
	public void createGui(Composite parent) {
	    comboViewer = new ComboViewer(parent, SWT.NONE);
	    Combo comboBox = comboViewer.getCombo();
	    comboBox.setBounds(0, 0, 26, 22);
	    comboBox.setText("Select");
	    comboViewer.setContentProvider(new ArrayContentProvider());
	    comboViewer.setInput(new RelationType[] {dummy});
	    comboViewer.setSelection(new StructuredSelection(dummy));
	    comboViewer.addSelectionChangedListener(this);
	}

	/* (non-Javadoc)
	 * @see com.minres.scviewer.e4.application.parts.PartListener#partActivated(org.eclipse.e4.ui.model.application.ui.basic.MPart)
	 */
	@Override
	public void partActivated(MPart part) {
		if(part.getObject() instanceof WaveformViewer){
			waveformViewerPart=(WaveformViewer) part.getObject();
			checkSelection(waveformViewerPart.getSelection());
		} else {
			waveformViewerPart=null;
			checkSelection(new StructuredSelection());
		}
	}

	/**
	 * Sets the selection.
	 *
	 * @param selection the selection
	 * @param partService the part service
	 */
	@Inject
	public void setSelection(@Named(IServiceConstants.ACTIVE_SELECTION) @Optional IStructuredSelection selection, EPartService partService){
		MPart part = partService.getActivePart();
		if(part!=null && part.getObject() instanceof WaveformViewer && comboViewer!=null){
			checkSelection(selection);
		}
	}

	/**
	 * Check selection.
	 *
	 * @param selection the selection
	 */
	protected void checkSelection(ISelection selection) {
		if( selection instanceof IStructuredSelection) {
			Object object= ((IStructuredSelection)selection).getFirstElement();			
			if(object instanceof ITx && waveformViewerPart!=null){
				comboViewer.getCombo().setEnabled(true);
				comboViewer.setInput(waveformViewerPart.getSelectionRelationTypes());//getAllRelationTypes());
			    comboViewer.setSelection(new StructuredSelection(waveformViewerPart.getRelationTypeFilter()));
			    return;
			}
		}
		comboViewer.getCombo().setEnabled(false);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.ISelectionChangedListener#selectionChanged(org.eclipse.jface.viewers.SelectionChangedEvent)
	 */
	@Override
	public void selectionChanged(SelectionChangedEvent event) {
		MPart part = partService.getActivePart();
		if(part!=null && part.getObject() instanceof WaveformViewer && !event.getSelection().isEmpty()){
			WaveformViewer waveformViewerPart=(WaveformViewer) part.getObject();
			if(event.getSelection() instanceof IStructuredSelection){
				waveformViewerPart.setNavigationRelationType(
						(RelationType)((IStructuredSelection)event.getSelection()).getFirstElement());
			}
		}
	}

}