// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlState.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Styling/AppStyle.h"
#endif

#define LOCTEXT_NAMESPACE "GitSourceControl.State"

int32 FGitSourceControlState::GetHistorySize() const
{
	return History.Num();
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::GetHistoryItem(int32 HistoryIndex) const
{
	check(History.IsValidIndex(HistoryIndex));
	return History[HistoryIndex];
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::FindHistoryRevision(int32 RevisionNumber) const
{
	for (const auto& Revision : History)
	{
		if (Revision->GetRevisionNumber() == RevisionNumber)
		{
			return Revision;
		}
	}

	return nullptr;
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::FindHistoryRevision(const FString& InRevision) const
{
	for (const auto& Revision : History)
	{
		if (Revision->GetRevision() == InRevision)
		{
			return Revision;
		}
	}

	return nullptr;
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3

ISourceControlState::FResolveInfo FGitSourceControlState::GetResolveInfo() const
{
	return PendingResolveInfo;
}

#else

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::GetBaseRevForMerge() const
{
	for (const auto& Revision : History)
	{
		// look for the the SHA1 id of the file, not the commit id (revision)
		if (Revision->FileHash == PendingMergeBaseFileHash)
		{
			return Revision;
		}
	}

	return nullptr;
}

#endif

#if ENGINE_MAJOR_VERSION == 5

FSlateIcon FGitSourceControlState::GetIcon() const
{
	switch (WorkingCopyState)
	{
	case EWorkingCopyState::Modified:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Subversion.CheckedOut");
	case EWorkingCopyState::Added:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Subversion.OpenForAdd");
	case EWorkingCopyState::Renamed:
	case EWorkingCopyState::Copied:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Subversion.Branched");
	case EWorkingCopyState::Deleted: // Deleted & Missing files does not show in Content Browser
	case EWorkingCopyState::Missing:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Subversion.MarkedForDelete");
	case EWorkingCopyState::Conflicted:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Subversion.NotAtHeadRevision");
	case EWorkingCopyState::NotControlled:
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Subversion.NotInDepot");
	case EWorkingCopyState::Unknown:
	case EWorkingCopyState::Unchanged: // Unchanged is the same as "Pristine" (not checked out) for Perforce, ie no icon
	case EWorkingCopyState::Ignored:
	default:
		break; //return FSlateIcon();
	}

	return FSlateIcon();
}

#else

// @todo add Slate icons for git specific states (NotAtHead vs Conflicted...)
FName FGitSourceControlState::GetIconName() const
{
	if(LockState == ELockState::Locked)
	{
		return FName("Subversion.CheckedOut");
	}
	else if(LockState == ELockState::LockedOther)
	{
		return FName("Subversion.CheckedOutByOtherUser");
	}
	else if (!IsCurrent())
	{
		return FName("Subversion.NotAtHeadRevision");
	}

	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Modified:
		if(bUsingGitLfsLocking)
		{
			return FName("Subversion.NotInDepot");
		}
		else
		{
			return FName("Subversion.CheckedOut");
		}
	case EWorkingCopyState::Added:
		return FName("Subversion.OpenForAdd");
	case EWorkingCopyState::Renamed:
	case EWorkingCopyState::Copied:
		return FName("Subversion.Branched");
	case EWorkingCopyState::Deleted: // Deleted & Missing files does not show in Content Browser
	case EWorkingCopyState::Missing:
		return FName("Subversion.MarkedForDelete");
	case EWorkingCopyState::Conflicted:
		return FName("Subversion.ModifiedOtherBranch");
	case EWorkingCopyState::NotControlled:
		return FName("Subversion.NotInDepot");
	case EWorkingCopyState::Unknown:
	case EWorkingCopyState::Unchanged: // Unchanged is the same as "Pristine" (not checked out) for Perforce, ie no icon
	case EWorkingCopyState::Ignored:
	default:
		return NAME_None;
	}

	return NAME_None;
}

FName FGitSourceControlState::GetSmallIconName() const
{
	if(LockState == ELockState::Locked)
	{
		return FName("Subversion.CheckedOut_Small");
	}
	else if(LockState == ELockState::LockedOther)
	{
		return FName("Subversion.CheckedOutByOtherUser_Small");
	}
	else if (!IsCurrent())
	{
		return FName("Subversion.NotAtHeadRevision_Small");
	}

	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Modified:
		if(bUsingGitLfsLocking)
		{
			return FName("Subversion.NotInDepot_Small");
		}
		else
		{
			return FName("Subversion.CheckedOut_Small");
		}
	case EWorkingCopyState::Added:
		return FName("Subversion.OpenForAdd_Small");
	case EWorkingCopyState::Renamed:
	case EWorkingCopyState::Copied:
		return FName("Subversion.Branched_Small");
	case EWorkingCopyState::Deleted: // Deleted & Missing files can appear in the Submit to Source Control window
	case EWorkingCopyState::Missing:
		return FName("Subversion.MarkedForDelete_Small");
	case EWorkingCopyState::Conflicted:
		return FName("Subversion.ModifiedOtherBranch_Small");
	case EWorkingCopyState::NotControlled:
		return FName("Subversion.NotInDepot_Small");
	case EWorkingCopyState::Unknown:
	case EWorkingCopyState::Unchanged: // Unchanged is the same as "Pristine" (not checked out) for Perforce, ie no icon
	case EWorkingCopyState::Ignored:
	default:
		return NAME_None;
	}

	return NAME_None;
}

#endif

FText FGitSourceControlState::GetDisplayName() const
{
	if (LockState == ELockState::Locked)
	{
		return LOCTEXT("Locked", "Locked For Editing");
	}
	else if (LockState == ELockState::LockedOther)
	{
		return FText::Format(LOCTEXT("LockedOther", "Locked by "), FText::FromString(LockUser));
	}
	else if (!IsCurrent())
	{
		return LOCTEXT("NotCurrent", "Not current");
	}

	switch (WorkingCopyState)
	{
	case EWorkingCopyState::Unknown:
		return LOCTEXT("Unknown", "Unknown");
	case EWorkingCopyState::Unchanged:
		return LOCTEXT("Unchanged", "Unchanged");
	case EWorkingCopyState::Added:
		return LOCTEXT("Added", "Added");
	case EWorkingCopyState::Deleted:
		return LOCTEXT("Deleted", "Deleted");
	case EWorkingCopyState::Modified:
		return LOCTEXT("Modified", "Modified");
	case EWorkingCopyState::Renamed:
		return LOCTEXT("Renamed", "Renamed");
	case EWorkingCopyState::Copied:
		return LOCTEXT("Copied", "Copied");
	case EWorkingCopyState::Conflicted:
		return LOCTEXT("ContentsConflict", "Contents Conflict");
	case EWorkingCopyState::Ignored:
		return LOCTEXT("Ignored", "Ignored");
	case EWorkingCopyState::NotControlled:
		return LOCTEXT("NotControlled", "Not Under Source Control");
	case EWorkingCopyState::Missing:
		return LOCTEXT("Missing", "Missing");
	}

	return FText();
}

FText FGitSourceControlState::GetDisplayTooltip() const
{
	if (LockState == ELockState::Locked)
	{
		return LOCTEXT("Locked_Tooltip", "Locked for editing by current user");
	}
	else if (LockState == ELockState::LockedOther)
	{
		return FText::Format(LOCTEXT("LockedOther_Tooltip", "Locked for editing by: {0}"), FText::FromString(LockUser));
	}
	else if (!IsCurrent())
	{
		return LOCTEXT("NotCurrent_Tooltip", "The file(s) are not at the head revision");
	}

	switch (WorkingCopyState)
	{
	case EWorkingCopyState::Unknown:
		return LOCTEXT("Unknown_Tooltip", "Unknown source control state");
	case EWorkingCopyState::Unchanged:
		return LOCTEXT("Pristine_Tooltip", "There are no modifications");
	case EWorkingCopyState::Added:
		return LOCTEXT("Added_Tooltip", "Item is scheduled for addition");
	case EWorkingCopyState::Deleted:
		return LOCTEXT("Deleted_Tooltip", "Item is scheduled for deletion");
	case EWorkingCopyState::Modified:
		return LOCTEXT("Modified_Tooltip", "Item has been modified");
	case EWorkingCopyState::Renamed:
		return LOCTEXT("Renamed_Tooltip", "Item has been renamed");
	case EWorkingCopyState::Copied:
		return LOCTEXT("Copied_Tooltip", "Item has been copied");
	case EWorkingCopyState::Conflicted:
		return LOCTEXT("ContentsConflict_Tooltip", "The contents of the item conflict with updates received from the repository.");
	case EWorkingCopyState::Ignored:
		return LOCTEXT("Ignored_Tooltip", "Item is being ignored.");
	case EWorkingCopyState::NotControlled:
		return LOCTEXT("NotControlled_Tooltip", "Item is not under version control.");
	case EWorkingCopyState::Missing:
		return LOCTEXT("Missing_Tooltip", "Item is missing (e.g., you moved or deleted it without using Git). This also indicates that a directory is incomplete (a checkout or update was interrupted).");
	}

	return FText();
}

const FString& FGitSourceControlState::GetFilename() const
{
	return LocalFilename;
}

const FDateTime& FGitSourceControlState::GetTimeStamp() const
{
	return TimeStamp;
}

// Deleted and Missing assets cannot appear in the Content Browser, but the do in the Submit files to Source Control window!
bool FGitSourceControlState::CanCheckIn() const
{
	if (bUsingGitLfsLocking)
	{
		return (((LockState == ELockState::Locked) && !IsConflicted()) || (WorkingCopyState == EWorkingCopyState::Added)) && IsCurrent();
	}
	else
	{
		return (WorkingCopyState == EWorkingCopyState::Added
			|| WorkingCopyState == EWorkingCopyState::Deleted
			|| WorkingCopyState == EWorkingCopyState::Missing
			|| WorkingCopyState == EWorkingCopyState::Modified
			|| WorkingCopyState == EWorkingCopyState::Renamed) && IsCurrent();
	}
}

bool FGitSourceControlState::CanCheckout() const
{
	if (bUsingGitLfsLocking)
	{
		// We don't want to allow checkout if the file is out-of-date, as modifying an out-of-date binary file will most likely result in a merge conflict
		return (WorkingCopyState == EWorkingCopyState::Unchanged || WorkingCopyState == EWorkingCopyState::Modified) && LockState == ELockState::NotLocked && IsCurrent();
	}
	else
	{
		return false; // With Git all tracked files in the working copy are always already checked-out (as opposed to Perforce)
	}
}

bool FGitSourceControlState::IsCheckedOut() const
{
	if (bUsingGitLfsLocking)
	{
		return LockState == ELockState::Locked;
	}
	else
	{
		return IsSourceControlled(); // With Git all tracked files in the working copy are always checked-out (as opposed to Perforce)
	}
}

bool FGitSourceControlState::IsCheckedOutOther(FString* Who) const
{
	if (Who != NULL)
	{
		*Who = LockUser;
	}
	return LockState == ELockState::LockedOther;
}

bool FGitSourceControlState::IsCurrent() const
{
	return !bNewerVersionOnServer;
}

bool FGitSourceControlState::IsSourceControlled() const
{
	return WorkingCopyState != EWorkingCopyState::NotControlled && WorkingCopyState != EWorkingCopyState::Ignored && WorkingCopyState != EWorkingCopyState::Unknown;
}

bool FGitSourceControlState::IsAdded() const
{
	return WorkingCopyState == EWorkingCopyState::Added;
}

bool FGitSourceControlState::IsDeleted() const
{
	return WorkingCopyState == EWorkingCopyState::Deleted || WorkingCopyState == EWorkingCopyState::Missing;
}

bool FGitSourceControlState::IsIgnored() const
{
	return WorkingCopyState == EWorkingCopyState::Ignored;
}

bool FGitSourceControlState::CanEdit() const
{
	return IsCurrent(); // With Git all files in the working copy are always editable (as opposed to Perforce)
}

bool FGitSourceControlState::CanDelete() const
{
	return !IsCheckedOutOther() && IsSourceControlled() && IsCurrent();
}

bool FGitSourceControlState::IsUnknown() const
{
	return WorkingCopyState == EWorkingCopyState::Unknown;
}

bool FGitSourceControlState::IsModified() const
{
	// Warning: for Perforce, a checked-out file is locked for modification (whereas with Git all tracked files are checked-out),
	// so for a clean "check-in" (commit) checked-out files unmodified should be removed from the changeset (the index)
	// http://stackoverflow.com/questions/12357971/what-does-revert-unchanged-files-mean-in-perforce
	//
	// Thus, before check-in UE Editor call RevertUnchangedFiles() in PromptForCheckin() and CheckinFiles().
	//
	// So here we must take care to enumerate all states that need to be commited,
	// all other will be discarded :
	//  - Unknown
	//  - Unchanged
	//  - NotControlled
	//  - Ignored
	return WorkingCopyState == EWorkingCopyState::Added
		|| WorkingCopyState == EWorkingCopyState::Deleted
		|| WorkingCopyState == EWorkingCopyState::Modified
		|| WorkingCopyState == EWorkingCopyState::Renamed
		|| WorkingCopyState == EWorkingCopyState::Copied
		|| WorkingCopyState == EWorkingCopyState::Missing
		|| WorkingCopyState == EWorkingCopyState::Conflicted;
}


bool FGitSourceControlState::CanAdd() const
{
	return WorkingCopyState == EWorkingCopyState::NotControlled;
}

bool FGitSourceControlState::IsConflicted() const
{
	return WorkingCopyState == EWorkingCopyState::Conflicted;
}

bool FGitSourceControlState::CanRevert() const
{
	return CanCheckIn();
}

TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::GetCurrentRevision() const
{
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
