/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Danny van Dyk
 *
 * This file is part of the EOS project. EOS is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * EOS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef EOS_GUARD_SRC_UTILS_SCAN_FILE_HH
#define EOS_GUARD_SRC_UTILS_SCAN_FILE_HH 1

#include <src/utils/exception.hh>
#include <src/utils/private_implementation_pattern.hh>
#include <src/utils/wrapped_forward_iterator.hh>

#include <vector>

namespace eos
{
    /*!
     * ScanFileError is parent to all exceptions thrown by ScanFile when creating, opening or accessing scan file data.
     */
    class ScanFileError :
        public Exception
    {
        public:
            /*!
             * Constructor.
             *
             * @param message The error message.
             */
            ScanFileError(const std::string & message);
    };

    /*!
     * ScanFileHDF5Error is thrown when an error occurs in interfacing libhdf5 during any
     * of ScanFile's operations.
     */
    class ScanFileHDF5Error :
        public ScanFileError
    {
        public:
            /*!
             * Constructor.
             *
             * @param function Name of the HDF5 function that failed.
             * @param code     Error code returned by the failing function.
             */
            ScanFileHDF5Error(const std::string & function, int code);
    };

    /*!
     * ScanFile represents an HDF5 formatted file that contains EOS scan data.
     *
     * An HDF5 DDL representation of a scan file follows:
     *
     * @code
     *   HDF5 "<FILE>" {
     *   GROUP "/" {
     *     GROUP "metadata" {
     *       DATASET "creator" {
     *         DATATYPE H5T_STRING {
     *           STRSIZE <CREATORLEN>
     *           STRPAD H5T_STR_NULLTERM
     *           CSET H5T_CSET_ASCII
     *           CTYPE H5T_C_S1
     *         }
     *         DATASPACE SCALAR
     *         DATA {
     *             "<CREATOR>"
     *         }
     *       }
     *       DATASET "eos_version" {
     *         DATATYPE H5T_STRING {
     *           STRSIZE <EOSVERSIONLEN>
     *           STRPAD H5T_STR_NULLTERM
     *           CSET H5T_CSET_ASCII
     *           CTYPE H5T_C_S1
     *         }
     *         DATASPACE SCALAR
     *         DATA {
     *             "<EOSVERSION>"
     *         }
     *       }
     *     }
     *     GROUP "data" {
     *       DATASET "<RESULT1>" {
     *         DATATYPE H5T_IEEE_F64LE
     *         DATASPACE SIMPLE { ( <TUPLES>, <ELEMENTS> ) / ( UNLIMITED, <ELEMENTS> ) }
     *         DATA {
     *           ...
     *         }
     *       }
     *       DATASET "<RESULT2>" {
     *         ...
     *       }
     *     }
     *   }
     *   }
     * @endcode
     *
     * Here, <FILE> is the filename, <TUPLES> is the number of scan tuples of <ELEMENTS> elements
     * each. <CREATOR> is an identifier of the creating program, and <EOSVERSION> is a string
     * representation of the EOS version/revision that was used to create the file.
     */
    class ScanFile :
        public PrivateImplementationPattern<ScanFile>
    {
        private:
            /// Constructor.
            ScanFile(Implementation<ScanFile> * imp);

        public:
            class DataSet;
            class Tuple;
            class WriteBuffer;

            ///@name Basic Functions
            ///@{
            /// Destructor.
            ~ScanFile();

            /*!
             * Named constructor to create a new file.
             *
             * @param filename   File which shall be created.
             * @param creator    Name of the creating program.
             * @param tuple_size Number of elements of each scan tuple.
             * @param scan_size  Number of scan tuples in file.
             */
            static ScanFile Create(const std::string & filename, const std::string & creator);

            /*!
             * Named constructor to open an existing file.
             *
             * @param filename File which shall be opened.
             */
            static ScanFile Open(const std::string & filename);
            ///@}

            ///@name Metadata Access
            ///@{
            /// Retrieve the file creator's name.
            const std::string & creator() const;

            /// Retrieve the file creator's version information.
            const std::string & eos_version() const;
            ///@}

            ///@name Data Access
            ///@{
            /*!
             * Retrieve a data set by its name.
             *
             * @param index Index of the scan tuple that shall be retrieved.
             */
            DataSet operator[] (const std::string & name);

            /*!
             * Create a new data set by name.
             *
             * @param name       Name of the new data set.
             * @param tuple_size Numer of elements in each tuple of the new data set.
             */
            DataSet add(const std::string & name, unsigned tuple_size);
            ///@}

            ///@name Iteration
            ///@{
            struct IteratorTag;
            typedef WrappedForwardIterator<IteratorTag, DataSet> Iterator;

            Iterator begin();
            Iterator end();
            ///@}
    };

    /*!
     * ScanFile::DataSet represents one of the data sets in the group "/data" within a ScanFile.
     */
    class ScanFile::DataSet :
        public PrivateImplementationPattern<ScanFile::DataSet>
    {
        private:
            DataSet(Implementation<ScanFile> * imp, const std::string & name);
            DataSet(Implementation<ScanFile> * imp, const std::string & name, unsigned tuple_size);

        public:
            friend class ScanFile;
            friend class Implementation<ScanFile>;
            friend ScanFile::DataSet & operator<< (ScanFile::DataSet &, const std::vector<double> &);
            friend ScanFile::DataSet & operator<< (ScanFile::DataSet &, const ScanFile::WriteBuffer &);

            ///@name Basic Functions
            ///@{
            /// Destructor.
            ~DataSet();
            ///@}

            ///@name Metadata Access
            ///@{
            /// Name of the data set
            std::string name() const;

            /// Number of columns in data set
            unsigned tuple_size() const;

            /// Number of rows in data set
            unsigned tuples() const;
            ///@}

            ///@name Data Access
            ///@{
            /*!
             * Retrieve a tuple element.
             *
             * @param index Index of the tuple element that shall be retrieved.
             */
            Tuple operator[] (const unsigned & index);
            ///@}
    };

    /*!
     * Append a tuple to a ScanFile::DataSet.
     *
     * @param lhs The ScanFile::DataSet to which shall be written.
     * @param rhs The tuple which shall be appended.
     */
    ScanFile::DataSet & operator<< (ScanFile::DataSet & lhs, const std::vector<double> & rhs);

    /*!
     * Append an entire write buffer to a ScanFile::DataSet.
     *
     * @param lhs The ScanFile::DataSet to which shall be written.
     * @param rhs The write buffer which shall be appended.
     */
    ScanFile::DataSet & operator<< (ScanFile::DataSet & lhs, const ScanFile::WriteBuffer & rhs);

    /*!
     * ScanFile::Tuple represents one of the scan tuples within a ScanFile.
     */
    class ScanFile::Tuple :
        public PrivateImplementationPattern<ScanFile::Tuple>
    {
        private:
            Tuple(const std::shared_ptr<Implementation<ScanFile::DataSet>> & imp, const unsigned & index);

        public:
            friend class ScanFile;

            ///@name Basic Functions
            ///@{
            /// Destructor.
            ~Tuple();
            ///@}

            ///@name Data Access
            ///@{
            /// Advance the scan tuple by one within the parent ScanFile.
            Tuple & operator++ ();

            /*!
             * Retrieve a tuple element.
             *
             * @param index Index of the tuple element that shall be retrieved.
             */
            double operator[] (const unsigned & index) const;
            ///@}
    };

    /*!
     * ScanFile::WriteBuffer temporarily keeps data which shall be written to one of the data sets within a ScanFile.
     */
    class ScanFile::WriteBuffer :
        public PrivateImplementationPattern<ScanFile::WriteBuffer>
    {
        public:
            friend ScanFile::DataSet & operator<< (ScanFile::DataSet & lhs, const ScanFile::WriteBuffer & rhs);
            friend ScanFile::WriteBuffer & operator<< (ScanFile::WriteBuffer &, const std::vector<double> &);

            ///@name Basic Functions
            ///@{
            /*!
             * Constructor.
             *
             * @param tuple_size Number of elements per recorded tuple.
             */
            WriteBuffer(const unsigned & tuple_size);

            /// Destructor.
            ~WriteBuffer();
            ///@}

            ///@name Access
            ///@{
            void clear();
            ///@}

            ///@name Metadata
            ///@{
            /// Retrieve the maximal number of tuples that can be stored in the buffer.
            unsigned capacity() const;

            /// Retrieve the number of tuples currently stored in the buffer.
            unsigned size() const;
            ///@}
    };

    /*!
     * Append a tuple to a ScanFile::WriteBuffer.
     *
     * @param lhs The ScanFile::WriteBuffer to which shall be written.
     * @param rhs The tuple which shall be appended.
     */
    ScanFile::WriteBuffer & operator<< (ScanFile::WriteBuffer & lhs, const std::vector<double> & rhs);
}

#endif
