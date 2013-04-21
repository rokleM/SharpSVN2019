#pragma once

#include "GitClientContext.h"

namespace SharpGit {

	public ref class GitSignature : public GitBase
	{
		literal __int64 DELTA_EPOCH_AS_FILETIME = 116444736000000000i64;
		DateTime _when;
		String ^_name;
		String ^_email;
		bool _readOnly;
		int _offset;

	internal:
		const git_signature * Alloc(GitPool ^pool)
		{
			git_signature *sig = (git_signature *)pool->Alloc(sizeof(*sig));

			// TODO: Store proper time in local tz

			sig->when.time = (When.ToFileTimeUtc() - DELTA_EPOCH_AS_FILETIME) / 10000000i64;
			sig->when.time -= 60 * _offset;
			sig->when.offset = _offset;

			sig->name = const_cast<char*>(pool->AllocString(Name));
			sig->email = const_cast<char*>(pool->AllocString(EmailAddress));

			return sig;
		}

	internal:
		GitSignature(const git_signature *from)
		{
			_name = from->name ? Utf8_PtrToString(from->name) : nullptr;
			_email = from->email ? Utf8_PtrToString(from->email) : nullptr;

			__int64 when = from->when.time + (__int64)(60 * from->when.offset); // Handle offset

			_when = DateTime::FromFileTimeUtc(when * 10000000i64 + DELTA_EPOCH_AS_FILETIME);
			_offset = from->when.offset;
			_readOnly = true;
		}

		GitSignature()
		{
			When = DateTime::UtcNow;
		}

	public:
		/// <summary>When the signature was added (as/auto-converted as GMT DateTime)</summary>
		property DateTime When
		{
			DateTime get()
			{
				return _when;
			}
			void set(DateTime value)
			{
				if (_readOnly)
					throw gcnew InvalidOperationException();

				// Always store as UTC
				if (value.Kind != System::DateTimeKind::Unspecified)
					_when = value.ToUniversalTime();
				else
					_when = DateTime(value.Ticks, System::DateTimeKind::Utc);

				// But also store the current offset at the time of setting
				{
					DateTime now = DateTime::Now;
					DateTime utcNow = now.ToUniversalTime();

					_offset = (int)Math::Round((now-utcNow).TotalMinutes);
				}
			}
		}

		property String^ Name
		{
			String^ get()
			{
				return _name;
			}
			void set(String ^value)
			{
				if (_readOnly)
					throw gcnew InvalidOperationException();

				_name = value;
			}
		}

		property String^ EmailAddress
		{
			String^ get()
			{
				return _email;
			}
			void set(String ^value)
			{
				if (_readOnly)
					throw gcnew InvalidOperationException();
				_email = value;
			}
		}

		property int TimeOffsetInMinutes
		{
			int get()
			{
				return _offset;
			}
		}
	};

	public ref class GitCommitArgs : public GitClientArgs
	{
		String ^_logMessage;
		String ^_updateReference;
		GitSignature ^_author;
		GitSignature ^_committer;

	public:
		GitCommitArgs()
		{
			UpdateReference = "HEAD";
			_author = gcnew GitSignature();
			_committer = gcnew GitSignature();
			DateTime now = DateTime::UtcNow;
			_author->When = now;
			_committer->When = now;
		}

	public:
		property String ^ LogMessage
		{
			String ^get()
			{
				return _logMessage;
			}
			void set(String ^value)
			{
				_logMessage = value;
			}
		}

		property String ^ UpdateReference
		{
			String^ get()
			{
				return _updateReference;
			}
			void set(String ^value)
			{
				_updateReference = value;
			}
		}

		property GitSignature^ Author
		{
			GitSignature^ get()
			{
				return _author;
			}
		}

		property GitSignature^ Committer
		{
			GitSignature^ get()
			{
				return _committer;
			}
		}
	};
}