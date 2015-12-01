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
package com.minres.scviewer.e4.application.internal.status;

import javax.inject.Inject;
import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.di.UISynchronize;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.jface.action.ContributionItem;
import org.eclipse.jface.action.StatusLineManager;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CLabel;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;

/**
 * The Class WaveStatusBarControl.
 */
public class WaveStatusBarControl extends StatusBarControl {

	/** The Constant ZOOM_LEVEL. */
	public static final String ZOOM_LEVEL="ZoomLevelUpdate";
	
	/** The Constant CURSOR_TIME. */
	public static final String CURSOR_TIME="CursorPosUpdate";
	
	/** The Constant MARKER_TIME. */
	public static final String MARKER_TIME="MarkerPosUpdate";
	
	/** The Constant MARKER_DIFF. */
	public static final String MARKER_DIFF="MarlerDiffUpdate";

	/** The model service. */
	@Inject
	EModelService modelService;

	/**
	 * The Class TextContributionItem.
	 */
	class TextContributionItem extends ContributionItem {

		/** The label string. */
		final String labelString;
		
		/** The width. */
		final int width;
		
		/** The text. */
		CLabel label, text;
		
		/** The content. */
		private String content;

		/**
		 * Instantiates a new text contribution item.
		 *
		 * @param labelString the label string
		 * @param width the width
		 */
		public TextContributionItem(String labelString, int width) {
			super();
			this.labelString = labelString;
			this.width=width;
			content="";
		}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.action.ContributionItem#fill(org.eclipse.swt.widgets.Composite)
		 */
		@Override
		public void fill(Composite parent) {
			Composite box=new Composite(parent, SWT.NONE);
			box.setLayout(new GridLayout(2, false));
			label=new CLabel(box, SWT.SHADOW_NONE);	
			label.setText(labelString);
			text=new CLabel(box, SWT.SHADOW_IN);
			GridData layoutData=new GridData(SWT.DEFAULT, SWT.DEFAULT, true, false);
			layoutData.minimumWidth=width;
			text.setLayoutData(layoutData);
		}

		/* (non-Javadoc)
		 * @see org.eclipse.jface.action.ContributionItem#isDynamic()
		 */
		@Override
		public boolean isDynamic() {
			return true;
		}

		/**
		 * Sets the text.
		 *
		 * @param message the new text
		 */
		public void setText(String message){
			this.content=message;
			if(text!=null && !text.isDisposed()) text.setText(content);
		}

	}

	/** The zoom contribution. */
	TextContributionItem cursorContribution, markerContribution, markerDiffContribution, zoomContribution;

	/**
	 * Instantiates a new wave status bar control.
	 *
	 * @param sync the sync
	 */
	@Inject
	public WaveStatusBarControl(UISynchronize sync) {
		super(sync);
		cursorContribution = new TextContributionItem("C:", 80);
		markerContribution = new TextContributionItem("M:", 80);
		markerDiffContribution = new TextContributionItem("C-M:", 80);
		zoomContribution = new TextContributionItem("Z:", 80);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP,cursorContribution);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP,markerContribution);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP,markerDiffContribution);
		manager.appendToGroup(StatusLineManager.BEGIN_GROUP, zoomContribution);
	}

	/**
	 * Sets the selection.
	 *
	 * @param selection the new selection
	 */
	@Inject
	public void setSelection(@Named(IServiceConstants.ACTIVE_SELECTION)@Optional IStructuredSelection selection){
		if(manager!=null && selection!=null){
			switch(selection.size()){
			case 0:
				manager.setMessage("");
				break;
			case 1:
				manager.setMessage(selection.getFirstElement().getClass().getSimpleName()+" selected");
				break;
			default:
				manager.setMessage(""+selection.size()+" Elements");
				break;
			}
		}
	}

	/**
	 * Gets the zoom event.
	 *
	 * @param text the text
	 * @return the zoom event
	 */
	@Inject @Optional
	public void  getZoomEvent(@UIEventTopic(ZOOM_LEVEL) String text) {
		zoomContribution.setText(text);
	} 

	/**
	 * Gets the cursor event.
	 *
	 * @param text the text
	 * @return the cursor event
	 */
	@Inject @Optional
	public void  getCursorEvent(@UIEventTopic(CURSOR_TIME) String text) {
		cursorContribution.setText(text);
	} 

	/**
	 * Gets the marker event.
	 *
	 * @param text the text
	 * @return the marker event
	 */
	@Inject @Optional
	public void  getMarkerEvent(@UIEventTopic(MARKER_TIME) String text) {
		markerContribution.setText(text);
	} 

	/**
	 * Gets the diff event.
	 *
	 * @param text the text
	 * @return the diff event
	 */
	@Inject @Optional
	public void  getDiffEvent(@UIEventTopic(MARKER_DIFF) String text) {
		markerDiffContribution.setText(text);
	} 

}